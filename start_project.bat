@echo off
IF NOT EXIST "venv" (
    echo Creating virtual environment...
    python -m venv venv
    echo Installing dependencies...
    CALL venv\Scripts\activate.bat
    @echo off
    IF NOT EXIST "requirements.txt" (
        echo Creating requirements.txt file...
        type nul > requirements.txt
    ) ELSE (
        echo requirements.txt file already exists.
        pip install -r requirements.txt
    )
) ELSE (
    echo Virtual environment already exists.
    echo Activating virtual environment...
    CALL venv\Scripts\activate.bat
)
