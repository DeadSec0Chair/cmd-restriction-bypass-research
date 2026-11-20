import os

while True:
    cmd = input(f"{os.getcwd()}> ")
    
    if cmd.strip().lower() == "exit":
        break
        
    if cmd.startswith("cd "):
        try:
            path = cmd[3:].strip()
            os.chdir(path)
        except FileNotFoundError:
            print("The system cannot find the path specified.")
        except Exception as e:
            print(e)
    else:
        os.system(cmd)




