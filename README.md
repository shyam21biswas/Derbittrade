# 🚀 Derbit trade

Derbittrade is a high-performance order execution and management system for trading on **Deribit Testnet**, developed in **C++**. This project is designed to run exclusively on **Linux and Ubuntu** 🐧 and requires **cURL** 🌐 for HTTP requests. 

## ✨ Features

- ⚡ **Real-time Trading Execution**: Supports order placement and management on the Deribit Testnet.
- 🚀 **High Performance**: Optimized for fast order execution.
- 🔒 **Secure API Communication**: Uses **cURL** for sending API requests.

## 📋 Prerequisites

To build and run this project, you need:

- 🖥 **Ubuntu/Linux** (Windows is not supported)
- 🛠 **C++ Compiler** (`g++` with C++11 or newer)
- 🌐 **cURL** (`libcurl` library)
- 🏦 **Deribit Testnet Account** (Sign up at [Deribit Testnet](https://test.deribit.com))

### 📦 Installing Dependencies

On **Ubuntu**, install the required dependencies:

```bash
sudo apt update
sudo apt install -y g++ curl libcurl4-openssl-dev
```

## 🏗 Building the Project

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/shyam21biswas/Derbittrade.git
   ```

2. **Navigate to the Project Directory**:

   ```bash
   cd Derbittrade
   ```

3. **Compile the Source Code**:

   ```bash
   g++ -std=c++11 -Iinclude -o derbittrade main.cpp -lcurl
   ```

## ▶️ Running the Application

After building the project, execute:

```bash
./derbittrade
```

## 🔧 Configuration

1. **Set up your Deribit API keys**:
   - You need to register on **Deribit Testnet** and generate API credentials.
   - Store your **Client ID** and **Client Secret** in a configuration file or environment variables.

2. **Authentication**:
   - The application will use **cURL** 🌐 to authenticate via the Deribit API.

## 🤝 Contributing

Contributions are welcome! To contribute:

1. 🍴 Fork the repository.
2. 🌱 Create a new branch (`feature-xyz` or `bugfix-xyz`).
3. 📝 Commit your changes with meaningful messages.
4. 🔄 Push your changes to your fork.
5. 📩 Submit a pull request with details about your changes.

## 📜 License

This project is licensed under the **MIT License**.

---

*Note: This README is based on the assumed structure of the repository. If you need further customization, please modify accordingly.*

