#!/usr/bin/env python3
import os
import re # Import the regular expressions module
import argparse

def transform_content_with_regex(file_content):
    """
    Uses regular expressions to find and replace the RemoteAddress
    and RemotePort blocks within the file's text content.
    """

    # Define the regex pattern:
    # - re.MULTILINE: Allows '^' to match the start of each line.
    # - re.DOTALL: Allows '.' to match newlines, which is crucial for `.*?`
    #   to span across the multi-line block.
    pattern = re.compile(
        r'^(\s*)'                               # Group 1: Capture the initial line indentation
        r'{\s*'                                 # Start of the first JSON object
        r'"name"\s*:\s*"RemoteAddress",\s*'     # The key "RemoteAddress"
        r'"value"\s*:\s*"([^"]+)"\s*'            # Group 2: Capture the IP address value
        r'}\s*,'                                # End of the first object and its comma
        r'.*?'                                  # Any character (including newlines/comments) until the next object
        r'^\s*{\s*'                             # Start of the second object on a new line
        r'"name"\s*:\s*"RemotePort",\s*'       # The key "RemotePort"
        r'"value"\s*:\s*"([^"]+)"\s*'            # Group 3: Capture the port value
        r'}',                                   # End of the second object
        flags=re.MULTILINE | re.DOTALL
    )

    def replacement_function(match):
        """
        This function is called for each match found by re.sub.
        It constructs the new text block for replacement.
        """
        indentation = match.group(1)
        ip_address = match.group(2)
        port_string = match.group(3)

        # Try to convert the port to a number; otherwise, keep it as a string
        try:
            port_output = int(port_string)
        except (ValueError, TypeError):
            port_output = f'"{port_string}"' # Keep quotes if it's not a number

        # Build the new JSON block, preserving the original indentation
        replacement_block = (
            f'{indentation}{{\n'
            f'{indentation}    "name": "Remote",\n'
            f'{indentation}    "value": [\n'
            f'{indentation}        "{ip_address}",\n'
            f'{indentation}        {port_output}\n'
            f'{indentation}    ]\n'
            f'{indentation}}}'
        )
        return replacement_block

    # Apply the substitution to the entire file content
    transformed_content, num_replacements = re.subn(pattern, replacement_function, file_content)
    
    return transformed_content, num_replacements > 0


def process_directory(root_directory):
    """
    Scans a directory to process the specified files.
    """
    if not os.path.isdir(root_directory):
        print(f"❌ Error: The path '{root_directory}' does not exist or is not a directory.")
        return

    print(f"Scanning directory: {root_directory}\n")
    valid_extensions = ('.json', '.jsonc') 
    for dirpath, _, filenames in os.walk(root_directory):
        for filename in filenames:
            if filename.endswith(valid_extensions):
                file_path = os.path.join(dirpath, filename)
                print(f"📄 Found file: {file_path}")

                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        original_content = f.read()

                    transformed_content, was_changed = transform_content_with_regex(original_content)

                    # Only rewrite the file if a change was made
                    if was_changed:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(transformed_content)
                        print("✅ Transformation completed successfully.")
                    else:
                        print("⚪ No changes needed.")

                except Exception as e:
                    print(f"❌ An error occurred while processing {file_path}: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Converts 'RemoteAddress'/'RemotePort' pairs in JSON-like files using regular expressions."
    )
    parser.add_argument(
        "path",
        type=str,
        help="The path to the directory to scan."
    )
    args = parser.parse_args()
    process_directory(args.path)

