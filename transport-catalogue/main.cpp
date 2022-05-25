#include <string>
#include<unordered_set>
#include<iostream>

#include"transport_catalogue.h"
#include"input_reader.h"
#include"stat_reader.h"

using namespace std;

int main() {
	transport_catalogue::TransportCatalogue newdatabase = input_reader::Load(cin);

	stat_reader::HandleRequests(newdatabase, cin, cout);
}