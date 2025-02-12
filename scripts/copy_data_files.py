import os
import shutil
Import("projenv")

def copy_data_files(source_dir, target_dir):
    if not os.path.exists(source_dir):
        print(f"Source directory {source_dir} does not exist.")
        return

    print(f"Copying files from {source_dir} to {target_dir}")
    
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    for item in os.listdir(source_dir):
        print("handle item:", item)
        s = os.path.join(source_dir, item)
        d = os.path.join(target_dir, item)
        if os.path.isdir(s):
            shutil.copytree(s, d, dirs_exist_ok=True)
        else:
            shutil.copy2(s, d)


#print("Env: " + str(projenv.Dump()))

# Define the source and target directories
source_dir = os.path.join(os.path.dirname(os.getcwd()), "data")
target_dir = os.path.join(projenv["PROJECT_DIR"], "data")


# Copy the files
copy_data_files(source_dir, target_dir)
