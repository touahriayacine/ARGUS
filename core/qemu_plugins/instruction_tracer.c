#define _GNU_SOURCE
#include <qemu-plugin.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

/*
 * Usage:
 *   -plugin ./trace.so,elf=/path/to/firmware.elf,out=/path/to/trace.txt
 *
 * Behavior:
 *   - Loads function symbols from elf=<...>
 *   - Logs each executed instruction in execution order
 *   - When disassembly is "NOP" followed only by spaces/tabs, stops logging
 *   - Flushes after each instruction to reduce data loss on brutal termination
 */

/* ---------------- ELF32 symbol loader (minimal) ---------------- */

#define EI_NIDENT 16
#define ELFCLASS32 1
#define ELFDATA2LSB 1

#define SHT_SYMTAB 2
#define STT_FUNC 2
#define ELF32_ST_TYPE(i) ((i) & 0x0f)

#pragma pack(push,1)
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr;

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
} Elf32_Sym;
#pragma pack(pop)

typedef struct {
    uint32_t addr;
    uint32_t size;
    char *name;
} FuncSym;

typedef struct {
    uint32_t pc;
    char *disas;
} InsnInfo;

static FuncSym *g_syms = NULL;
static size_t g_syms_n = 0;
static int g_have_syms = 0;

static FILE *g_out = NULL;
static const char *g_last_func = NULL;

/* Plugin parameters */
static const char *g_elf_path = NULL;
static const char *g_out_path = NULL;

/* Global stop flag once NOP is executed */
static int g_stop_tracing = 0;

static int cmp_sym_addr(const void *a, const void *b)
{
    const FuncSym *x = (const FuncSym *)a;
    const FuncSym *y = (const FuncSym *)b;

    if (x->addr < y->addr) return -1;
    if (x->addr > y->addr) return 1;
    return 0;
}

static char *read_file_range(FILE *f, uint32_t off, uint32_t size)
{
    if (fseek(f, (long)off, SEEK_SET) != 0) {
        return NULL;
    }

    char *buf = (char *)malloc(size ? size : 1);
    if (!buf) {
        return NULL;
    }

    if (size && fread(buf, 1, size, f) != size) {
        free(buf);
        return NULL;
    }

    return buf;
}

static void free_syms(void)
{
    if (!g_syms) {
        return;
    }

    for (size_t i = 0; i < g_syms_n; i++) {
        free(g_syms[i].name);
    }

    free(g_syms);
    g_syms = NULL;
    g_syms_n = 0;
    g_have_syms = 0;
}

static void load_elf_symbols(const char *elf_path)
{
    free_syms();

    FILE *f = fopen(elf_path, "rb");
    if (!f) {
        fprintf(stderr, "plugin: could not open ELF '%s': %s\n",
                elf_path, strerror(errno));
        return;
    }

    Elf32_Ehdr eh;
    if (fread(&eh, 1, sizeof(eh), f) != sizeof(eh)) {
        fclose(f);
        return;
    }

    if (eh.e_ident[0] != 0x7f || eh.e_ident[1] != 'E' ||
        eh.e_ident[2] != 'L' || eh.e_ident[3] != 'F' ||
        eh.e_ident[4] != ELFCLASS32 ||
        eh.e_ident[5] != ELFDATA2LSB) {
        fprintf(stderr, "plugin: unsupported ELF\n");
        fclose(f);
        return;
    }

    if (eh.e_shoff == 0 || eh.e_shnum == 0) {
        fclose(f);
        return;
    }

    if (fseek(f, eh.e_shoff, SEEK_SET) != 0) {
        fclose(f);
        return;
    }

    Elf32_Shdr *sh = malloc(eh.e_shnum * sizeof(Elf32_Shdr));
    if (!sh) {
        fclose(f);
        return;
    }

    if (fread(sh, sizeof(Elf32_Shdr), eh.e_shnum, f) != eh.e_shnum) {
        free(sh);
        fclose(f);
        return;
    }

    int symtab_idx = -1;
    for (uint16_t i = 0; i < eh.e_shnum; i++) {
        if (sh[i].sh_type == SHT_SYMTAB &&
            sh[i].sh_entsize == sizeof(Elf32_Sym)) {
            symtab_idx = (int)i;
            break;
        }
    }

    if (symtab_idx < 0) {
        fprintf(stderr, "plugin: no .symtab found\n");
        free(sh);
        fclose(f);
        return;
    }

    int strtab_idx = (int)sh[symtab_idx].sh_link;
    if (strtab_idx < 0 || strtab_idx >= eh.e_shnum) {
        free(sh);
        fclose(f);
        return;
    }

    char *strtab = read_file_range(f, sh[strtab_idx].sh_offset, sh[strtab_idx].sh_size);
    Elf32_Sym *syms = (Elf32_Sym *)read_file_range(
        f, sh[symtab_idx].sh_offset, sh[symtab_idx].sh_size);

    if (!strtab || !syms) {
        free(strtab);
        free(syms);
        free(sh);
        fclose(f);
        return;
    }

    size_t sym_n = sh[symtab_idx].sh_size / sizeof(Elf32_Sym);

    g_syms = malloc(sym_n * sizeof(FuncSym));
    if (!g_syms) {
        free(strtab);
        free(syms);
        free(sh);
        fclose(f);
        return;
    }

    g_syms_n = 0;

    for (size_t i = 0; i < sym_n; i++) {
        if (ELF32_ST_TYPE(syms[i].st_info) != STT_FUNC) {
            continue;
        }

        if (!syms[i].st_value || !syms[i].st_name) {
            continue;
        }

        const char *nm = strtab + syms[i].st_name;
        if (!nm || !*nm) {
            continue;
        }

        g_syms[g_syms_n].addr = syms[i].st_value;
        g_syms[g_syms_n].size = syms[i].st_size;
        g_syms[g_syms_n].name = strdup(nm);

        if (g_syms[g_syms_n].name) {
            g_syms_n++;
        }
    }

    free(strtab);
    free(syms);
    free(sh);
    fclose(f);

    if (!g_syms_n) {
        free_syms();
        return;
    }

    qsort(g_syms, g_syms_n, sizeof(FuncSym), cmp_sym_addr);
    g_have_syms = 1;

    fprintf(stderr, "plugin: loaded %zu function symbols from %s\n",
            g_syms_n, elf_path);
}

static const char *lookup_func(uint32_t pc)
{
    if (!g_have_syms || !g_syms_n) {
        return NULL;
    }

    size_t lo = 0;
    size_t hi = g_syms_n;

    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (g_syms[mid].addr <= pc) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (lo == 0) {
        return NULL;
    }

    return g_syms[lo - 1].name;
}

static int is_nop_disas(const char *dis)
{
    if (!dis) {
        return 0;
    }

    if (strncmp(dis, "NOP", 3) != 0) {
        return 0;
    }

    for (const char *p = dis + 3; *p != '\0'; ++p) {
        if (*p != ' ' && *p != '\t') {
            return 0;
        }
    }

    return 1;
}
static unsigned int g_consecutive_nops = 0;
static const unsigned int g_nop_limit = 10;

static void insn_exec_cb(unsigned int cpu_index, void *userdata)
{
    (void)cpu_index;

    InsnInfo *ii = (InsnInfo *)userdata;
    if (!ii || !g_out || g_stop_tracing) {
        return;
    }

    const char *fn = lookup_func(ii->pc);
    if (fn && fn != g_last_func) {
        fprintf(g_out, "\nIN: %s\n", fn);
        g_last_func = fn;
    }

    fprintf(g_out, "%08x: %s\n", ii->pc, ii->disas);
    fflush(g_out);

    if (is_nop_disas(ii->disas)) {
        g_consecutive_nops++;

        fprintf(stderr,
                "plugin: consecutive NOP #%u at 0x%08x\n",
                g_consecutive_nops, ii->pc);

        if (g_consecutive_nops >= g_nop_limit) {
            fprintf(stderr,
                    "plugin: reached %u consecutive NOPs, stopping trace\n",
                    g_nop_limit);
            fflush(g_out);
            g_stop_tracing = 1;
        }
    } else {
        g_consecutive_nops = 0;
    }
}

static void tb_trans_cb(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    (void)id;

    if (g_stop_tracing) {
        return;
    }

    size_t n = qemu_plugin_tb_n_insns(tb);
    if (n == 0) {
        return;
    }

    for (size_t i = 0; i < n; i++) {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
        if (!insn) {
            continue;
        }

        const char *dis = qemu_plugin_insn_disas(insn);
        if (!dis) {
            dis = "<no-disas>";
        }

        InsnInfo *ii = malloc(sizeof(*ii));
        if (!ii) {
            continue;
        }

        ii->pc = (uint32_t)qemu_plugin_insn_vaddr(insn);
        ii->disas = strdup(dis);
        if (!ii->disas) {
            free(ii);
            continue;
        }

        qemu_plugin_register_vcpu_insn_exec_cb(
            insn,
            insn_exec_cb,
            QEMU_PLUGIN_CB_NO_REGS,
            ii);
    }
}

static void plugin_exit(qemu_plugin_id_t id, void *p)
{
    (void)id;
    (void)p;

    if (g_out) {
        fflush(g_out);
        fclose(g_out);
        g_out = NULL;
    }

    free_syms();
}

QEMU_PLUGIN_EXPORT int qemu_plugin_install(
    qemu_plugin_id_t id,
    const qemu_info_t *info,
    int argc,
    char **argv)
{
    (void)info;

    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "elf=", 4) == 0) {
            g_elf_path = argv[i] + 4;
        } else if (strncmp(argv[i], "out=", 4) == 0) {
            g_out_path = argv[i] + 4;
        }
    }

    if (!g_elf_path) {
        fprintf(stderr, "plugin: missing required argument elf=<path>\n");
        return 1;
    }

    if (!g_out_path) {
        fprintf(stderr, "plugin: missing required argument out=<path>\n");
        return 1;
    }

    g_out = fopen(g_out_path, "wb");
    if (!g_out) {
        fprintf(stderr, "plugin: cannot open %s: %s\n",
                g_out_path, strerror(errno));
        return 1;
    }

    /*
     * Keep it unbuffered to minimize loss if QEMU is killed brutally.
     * If performance is too slow, change to full buffering and keep fflush().
     */
    setvbuf(g_out, NULL, _IONBF, 0);

    load_elf_symbols(g_elf_path);

    qemu_plugin_register_vcpu_tb_trans_cb(id, tb_trans_cb);
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);

    return 0;
}