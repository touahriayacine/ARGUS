import json

def load_configuration(mcu_name,json_path):
    with open(json_path, "r") as file:
        data = json.load(file)

    mcu_config = next((item for item in data if item["mcu_name"] == mcu_name),None)
    return mcu_config
    