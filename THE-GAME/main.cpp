#include "MainWindow.h"
#include <QApplication>
#include "NetworkManager.h" //temp
#include <iostream> //temp

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	//Game g(3);
	MainWindow w;
	//w.setGame(&g);
	w.show();

    // Test rapid în consolă
    


    NetworkManager netManager("http://localhost:18080");



	bool lobbyCreated = false;
	bool gameStarted = false;

	while (!lobbyCreated) {
		std::cout << "Register or Login? r/l" << std::endl;
		char choice;
		std::cin >> choice;
		if (choice == 'r') {
			std::cout << "Registering: Username: " << std::endl;
			std::string username, password;
			std::cin >>  username;
			std::cout << "Password: " << std::endl;
			std::cin >> password;
			auto reg = netManager.registerUser(username, password);
			std::cout << (reg.success ? "Registration OK" : "Registration FAILED") << std::endl;
			if (!reg.success) {
				std::cout << "Error: " << reg.error << std::endl;
			}
			continue;
		}
		std::cout << "Logging In: Username: " << std::endl;
		std::string username, password;
		std::cin >> username;
		std::cout << "Password: " << std::endl;
		std::cin >> password;
		auto login = netManager.loginUser(username, password);
		std::cout << (login.success ? "Login OK" : "Login FAILED") << std::endl;
		std::cout << "Exit login loop? y/n" << std::endl;
		char c;
		std::cin >> c;
		if (c == 'y') {
			lobbyCreated = true;
		}
	}

	while (lobbyCreated) {
		//TODO
	}

	while (gameStarted) {
		//TODO
	}
    


	//de implementat o functie de while in timpul rularii jocului cu  o functie de polling pt lobby status 
	//in timpul jocului aceasta ruleaza in background si actualizeaza interfata cand e cazul (validare la o actiune) si trimite cereri cand e cazul (de ex chat)
	//cu NetworkManager ca wrapper pt server api
    

	

	return app.exec();
}