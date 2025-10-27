#include <filesystem>
#include <fstream>
#include <iostream>

int main() {
    std::filesystem::path path = std::filesystem::current_path() / "config_exemplo.txt";

    std::cout << "CWD = " << std::filesystem::current_path() << "\n";
    std::cout << "Tentando abrir arquivo: " << path << "\n";

    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::perror("Erro ao abrir o arquivo");
        return 1;
    }

    std::cout << "Arquivo aberto com sucesso!\n";

    std::string linha;
    while (std::getline(file, linha)) {
        std::cout << "Linha lida: " << linha << "\n";
    }

    file.close();
    return 0;
}
