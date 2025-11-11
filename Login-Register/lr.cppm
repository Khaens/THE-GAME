export module lr;
export import std;
export class __declspec(dllexport) UserManager
{
public:
	UserManager(const std::string& dbPath);

	void Register(const std::string& username, const std::string& password);
	void Login(const std::string& username, const std::string& password);

	bool UserExists(const std::string& username);
private:
	
};