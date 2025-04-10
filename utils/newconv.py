import os
import binascii

def convert_files_to_header(folder_path, output_header="webfiles.h"):
    if not os.path.exists(folder_path):
        print(f"Folder '{folder_path}' does not exist.")
        return
    
    with open(output_header, "w", encoding="utf-8") as header_file:
        header_file.write("#pragma once\n#include <pgmspace.h>\n\n")
        header_file.write("struct WebFile {\n    const char* filename;\n    const unsigned char* data;\n    unsigned int length;\n};\n\n")
        
        file_entries = []
        
        for root, _, files in os.walk(folder_path):
            for filename in files:
                file_path = os.path.join(root, filename)
                relative_path = os.path.relpath(file_path, folder_path).replace("\\", "/")
                relative_path_cleaned = relative_path[:-3] if relative_path.endswith(".gz") else relative_path
                var_name = os.path.splitext(relative_path_cleaned)[0].replace("/", "_").replace("-", "_").replace(".", "_")
                var_name += os.path.splitext(relative_path_cleaned)[1].replace("/", "_").replace("-", "_").replace(".", "_")
                var_name = "f_" + "_".join(var_name.split("_")[-2:])  # Shorten name but keep it recognizable
                
                with open(file_path, "rb") as f:
                    binary_content = f.read()
                    hex_content = binascii.hexlify(binary_content).decode("utf-8")
                    
                    header_file.write(f"const PROGMEM unsigned char {var_name}[] = {{\n")
                    for i in range(0, len(hex_content), 2):
                        if i % 32 == 0:
                            header_file.write("    ")  # Indentation for readability
                        header_file.write(f"0x{hex_content[i]}{hex_content[i+1]}, ")
                        if (i + 2) % 32 == 0 or i + 2 >= len(hex_content):
                            header_file.write("\n")
                    
                    header_file.write(f"}};\nconst unsigned int {var_name}_len = {len(binary_content)};\n\n")
                    
                    file_entries.append(f"    {{ \"/{relative_path_cleaned}\", {var_name}, {var_name}_len }}")
        
        header_file.write("const WebFile webFiles[] = {\n" + ",\n".join(file_entries) + "\n};\n")
        header_file.write("const unsigned int webFilesCount = sizeof(webFiles) / sizeof(WebFile);\n\n")
    
    print(f"Header file '{output_header}' generated successfully.")

# Example usage
convert_files_to_header("web")  # Change "web" to your folder name
