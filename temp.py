from genericpath import isdir, isfile
import json
import os

directory = input('Directory path: ')
prefix = input('Prefix: ')

file_dict = dict()

for file in os.listdir(directory):
    if os.path.isdir(os.path.join(directory, file)):
        continue

    file_base_name = os.path.basename(file)
    file_base_name_excluding_ext = '.'.join(file_base_name.split('.')[:-1])

    file_dict[file_base_name] = f'{prefix}{file_base_name_excluding_ext}'

print(json.dumps(file_dict, indent=True))