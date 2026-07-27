# Debian 12 base
FROM debian:12

# Avoid prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install base tools and languages
RUN apt-get update && apt-get install -y \
    sudo git curl wget \
    build-essential gcc g++ clang gdb \
    cmake ninja-build unzip zip \
    python3 python3-pip python3-venv \
    libfreetype6-dev \
    libfontconfig1-dev \
    postgresql-server-dev-15 \
    libpq-dev \
    openjdk-17-jdk \
    apt-transport-https gnupg \
 && rm -rf /var/lib/apt/lists/*

# Install latest .NET 8 LTS SDK
RUN wget https://packages.microsoft.com/config/debian/12/packages-microsoft-prod.deb \
 && dpkg -i packages-microsoft-prod.deb \
 && rm packages-microsoft-prod.deb \
 && apt-get update && apt-get install -y dotnet-sdk-6.0 \
 && rm -rf /var/lib/apt/lists/*

# Create non-root user (same as VS Code default)
RUN useradd -m vscode && echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Create Python virtual environment for user
RUN python3 -m venv /opt/venv \
 && /opt/venv/bin/pip install --upgrade pip \
 && /opt/venv/bin/pip install \
        wheel \
        setuptools \
        waitress \
        flasgger \
        psycopg2 \
        sqlalchemy \
        numpy \
        celery \
        marshmallow \
        redis \
        flask_httpauth \
        pyparsing \
        requests

# Install PowerShell
RUN apt-get update && \
    apt-get install -y wget apt-transport-https software-properties-common && \
    wget -q https://packages.microsoft.com/config/debian/12/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    apt-get update && \
    apt-get install -y powershell && \
    rm -rf /var/lib/apt/lists/* && \
    ln -s /usr/bin/pwsh /usr/bin/powershell

# Ensure venv is used by default for all users
ENV PATH="/opt/venv/bin:$PATH"

# Set work directory
WORKDIR /workspaces

# Default to non-root user
USER vscode
