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


def load_env_file(env_path):
    """Lädt Umgebungsvariablen aus einer .env Datei"""
    if not os.path.exists(env_path):
        print(f"{env_path} existiert nicht.")
        return
    with open(env_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                if "#" in line:
                    line = line.split("#", 1)[0].strip()
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip().strip('"').strip("'")
                print(f"Lade Umgebungsvariable: {key}={value}")
                os.environ[key] = value

# Check for .env files in the main project directory and load variables if found
main_dir = projenv["PROJECT_DIR"]

env_files = [f for f in os.listdir(main_dir) if f.endswith(".env")]

for env_file in env_files:
    env_path = os.path.join(main_dir, env_file)
    print(f"Lade Umgebungsvariablen aus: {env_path}")
    load_env_file(env_path)



#print("Env: " + str(projenv.Dump()))

# Define the source and target directories
source_dir = os.path.join(os.path.dirname(os.getcwd()), "data")
if os.getenv("HTML_DIR"):
    target_dir = os.path.join(projenv["PROJECT_DIR"], os.getenv("HTML_DIR"))
else:
    target_dir = projenv["PROJECT_DATA_DIR"]

# Copy the files
copy_data_files(source_dir, target_dir)
