#!/usr/bin/env bash

function check_brew_installed() {
    if command -v brew >/dev/null 2>&1; then
        echo "Homebrew is installed. continuing..."
    else
        echo "Homebrew is NOT installed."
        ask_install_brew
    fi
}

function ask_install_brew() {
    echo "Do you want to install Homebrew? (yes/no)"
    read user_input

    if [[ $user_input == "yes" ]]; then
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        install_python
    elif [[ $user_input == "no" ]]; then
        echo "Exiting without installing Homebrew."
        exit 1
    else
        echo "Invalid input. Please type 'yes' or 'no'."
        ask_install_brew
    fi
}

function install_python() {
    echo "Installing Python 3.11 or higher..."
    brew install python@3.11
    echo "Python 3.11 or higher has been installed."
    start_python_venv
}

function check_python_version() {
    if command -v python3 >/dev/null 2>&1; then
        version=$(python3 --version 2>&1 | awk '{print $2}')
        if [[ $(echo "$version >= 3.11" | bc -l) -eq 1 ]]; then
            echo "Python 3.11 or higher is installed."
            start_python_venv
        else
            echo "Python 3.11 or higher is NOT installed."
            check_brew_installed
        fi
    else
        echo "Python 3 is NOT installed."
        check_brew_installed
    fi
}


function run_start_script() {
    if [ -f "./start.sh" ]; then
        echo "Running start.sh..."
        chmod +x ./start.sh
        ./start.sh
    else
        echo "start.sh does not exist in the current directory."
    fi
}

function start_python_venv() {
    echo "Creating a Python virtual environment..."
    python3 -m venv venv
    echo "Activating the virtual environment..."
    source venv/bin/activate
    install_python_dependencies

}

function install_python_dependencies() {
    if [ -f "requirements.txt" ]; then
        echo "Installing Python dependencies..."
        pip install -r requirements.txt
        check_and_create_credentials

        echo " now go read the README.md file to see how to fill the credentials.json file"
    else
        echo "requirements.txt does not exist in the current directory."
    fi
}

function run_python_main() {
    if [ -f "main.py" ]; then
        echo "Running main.py..."
        python main.py
    else
        echo "main.py does not exist in the current directory."
    fi
}

function check_and_create_credentials() {
    if [ ! -f "credentials.json" ]; then
        echo "Creating credentials.json..."
        cat << EOF > credentials.json
{
    "harvest":{
        "token": "<replace this >",
        "account_id": "<replace this >"
    },
    "gmail":{
        "user" : "<replace this >",
        "password" : "<replace this >"
    }
}
EOF
    fi
}

function check_and_delete_dir_usr_local_bin() {
    if [ -d "/usr/local/bin" ]; then
        if [ "$(ls -A /usr/local/bin)" ]; then
            echo "/usr/local/bin is not empty."
            sudo chown -R $(whoami) /usr/local/bin
        else
            echo "/usr/local/bin is empty. Deleting..."
            rmdir /usr/local/bin
        fi
    else
        echo "/usr/local/bin does not exist."
    fi
}


check_and_delete_dir_usr_local_bin


check_python_version

