import argparse


def parse_arguments():

    parser = argparse.ArgumentParser(description="Process ")

    parser.add_argument(
        "--project-path",
        type=str,
        required=True,
        help="your project full path"
    )

    parser.add_argument(
        "--output-path",
        type=str,
        required=False,
        default="build",
        help="your output path"
    )

    parser.add_argument(
        "--mcu",
        type=str,
        required=True,
        help="your microcontroller name: [arduino_uno, esp32c3]"
    )

    parser.add_argument(
        "--save-counts",
        type=bool,
        default=True,
        help="save the instruction counts into a csv file"
    )

    args = parser.parse_args()
    
    return args.project_path, args.output_path, args.mcu , args.save_counts