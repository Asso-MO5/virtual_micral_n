import glob
import json
import os

CONFIGURATION_FILE_NAME = "documentation.json"


def get_configuration(script_path):
    try:
        with open(os.path.join(script_path, CONFIGURATION_FILE_NAME)) as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"The configuration file `{CONFIGURATION_FILE_NAME}` could not be found.")


def resolve_root_path(configuration):
    root_path = configuration["root_path"]
    root_path = os.path.normpath(os.path.join(configuration["script_path"], root_path))
    configuration["root_path"] = root_path


def resolve_markdown_inputs(configuration):
    input_files = []
    for pattern in configuration["markdown_files"]:
        adjusted_pattern = os.path.join(configuration["root_path"], pattern)
        input_files.extend(list(glob.glob(adjusted_pattern, recursive=True)))

    configuration["input_files"] = input_files


def validate_files(root_path, files):
    for file in set(files):
        complete_path = os.path.join(root_path, file)
        if os.path.exists(complete_path):
            print(f"{complete_path} exists")
        else:
            print(f"{complete_path} DOES NOT EXIST")


def parse_documentation_files(configuration):
    import collector
    collector = collector.Collector()

    root_path_prefix_length = len(configuration["root_path"]) + 1
    for file in configuration["input_files"]:
        simple_file_name = file[root_path_prefix_length:]
        print(f"Parsing {simple_file_name}")

        base_dir = os.path.dirname(simple_file_name)
        print(f"In folder {base_dir}")

        with open(file, "r", encoding="UTF-8") as f:
            collector.collect(base_dir, f.read())

    validate_files(configuration["root_path"], collector.get_references().values())
    # for code, file in collector.get_references():
    #     print(code, file)


def make_documentation():
    script_path = os.path.dirname(os.path.realpath(__file__))

    configuration = get_configuration(script_path)
    configuration["script_path"] = script_path
    resolve_root_path(configuration)
    resolve_markdown_inputs(configuration)

    print(f"Script path: {configuration['script_path']}")
    print(f"Root path: {configuration['root_path']}")
    print(f"All Markdown files: {configuration['input_files']}")

    parse_documentation_files(configuration)


if __name__ == '__main__':
    make_documentation()
