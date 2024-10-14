#!/bin/sh

# 1. Download the LLVM package
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.1/LLVM-19.1.1-Linux-X64.tar.xz

# 2. Extract the archive (this will create a directory named LLVM-19.1.1-Linux-X64)
tar -xvf LLVM-19.1.1-Linux-X64.tar.xz

# 3. Move the extracted directory to /opt (optional, but recommended for system-wide installation)
sudo mv LLVM-19.1.1-Linux-X64 /opt/llvm-19.1.1

# 4. Create symbolic links to the clang-format binary
sudo ln -s /opt/llvm-19.1.1/bin/clang-format /usr/local/bin/clang-format

# 5. Update your PATH (add this line to your ~/.bashrc or ~/.zshrc file)
echo 'export PATH=$PATH:/opt/llvm-19.1.1/bin' >> ~/.bashrc

# 6. Reload your shell configuration
source ~/.bashrc

rm LLVM-19.1.1-Linux-X64.tar.xz

# 7. Verify the installation
clang-format --version