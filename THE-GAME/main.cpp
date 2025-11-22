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
    /*
    NetworkManager netManager("http://localhost:18080");

    std::cout << "Testing register..." << std::endl;
    auto reg = netManager.registerUser("test", "pass");
    std::cout << (reg.success ? "OK" : "FAILED") << std::endl;

    std::cout << "Testing login..." << std::endl;
    auto login = netManager.loginUser("test", "pass");
    std::cout << (login.success ? "OK" : "FAILED") << std::endl;
    */



	//de implementat o functie de while in timpul rularii jocului cu  o functie de polling pt lobby status 
	//in timpul jocului aceasta ruleaza in background si actualizeaza interfata cand e cazul (validare la o actiune) si trimite cereri cand e cazul (de ex chat)
	//cu NetworkManager ca wrapper pt server api
    

	

	return app.exec();
}