#include <iostream>
#include <vector>
using namespace std;

class Airport {
	private:
		string code;
	public:
		vector <Airport*> connections;
		Airport(const string& acode);
		void addFlight(Airport* airport);
		const string getCode() const;
		bool hasDirect(Airport* airport);
};

class AirportNetwork {
	private:
		vector <Airport*> airports;
	public:
		Airport* findAirport(const string& code);
		void addAirport(const string& code);
		void addConnection(const string& code1, const string& code2);
		void printAirportNetwork();
};

int main() {
	AirportNetwork network;

	network.addAirport("ATL");
	network.addAirport("LAX");
	network.addAirport("JFK");
	network.addAirport("ORD");
	network.addAirport("TYS");
	network.addAirport("BNA");

	network.addConnection("ATL", "LAX");
	network.addConnection("ATL", "JFK");
	network.addConnection("LAX", "ORD");
	network.addConnection("JFK", "ORD");
	network.addConnection("TYS", "ATL");
	network.addConnection("BNA", "JFK");

	network.printAirportNetwork();

	string code1, code2;
	cout << "Enter a 3 character airport code for departure: ";
	cin >> code1;
	cout << "Enter a 3 character airport code for arrival: ";
	cin >> code2;
	Airport *a1 = network.findAirport(code1);
	Airport *a2 = network.findAirport(code2);
	if (a1->hasDirect(a2)) cout << "You can fly direct from " << code1 << " to " << code2 << "." << endl;
	else cout << "You cannot fly direct from " << code1 << " to " << code2 << "." << endl;

	return 0;
}

Airport::Airport(const string& acode) {
	/* TODO: input the Airport object with the given code*/
	code  = acode;
}

void Airport::addFlight(Airport* airport) {
	/* TODO: add a flight to a neighboring airport from the current airport (i.e. connections) */
	connections.push_back(airport);
}

const string Airport::getCode() const {
	/* TODO: return the 3 letter code */
	return code;
}

bool Airport::hasDirect(Airport* airport) {
	/* TODO: return T/F if there is a direct flight to the parameter */
	for (Airport* connection : connections) {
		if (connection->code == airport->code) {
			return true;
		}
	}
	return false;
}

Airport* AirportNetwork::findAirport(const string& code) {
	/* TODO: find the airport object that has the given code */
for (Airport *airport : airports) {
	if (airport->getCode() == code) {
		return airport;
	}
}
return NULL;
}

void AirportNetwork::addAirport(const string& code) {
	/* TODO: this should create a new Airport object with the given code and add it to the airport network  */
	airports.push_back(new Airport(code));
	//or Airport *a = new Airport(code);
	//airports.push_back();
}

void AirportNetwork::addConnection(const string& code1, const string& code2) {
	/* TODO: add a connection betweent the two airports */
	Airport* a1 = findAirport(code1);
	Airport* a2 = findAirport(code2);

	a1->addFlight(a2);
	a2->addFlight(a1);
}

void AirportNetwork::printAirportNetwork() {
	/* TODO: print everything nicely */
	cout << "Airport Network: " << endl;
	for (Airport* airport : airports) {
		cout << airport->getCode() << "-> Connections: ";
		for (int i = 0; i < airport->connections.size(); i++) {
			cout << airport->connections[i]->getCode() << " ";
			//Airport *a = airport->connections[i]
		}
		cout << endl;
	}
}

