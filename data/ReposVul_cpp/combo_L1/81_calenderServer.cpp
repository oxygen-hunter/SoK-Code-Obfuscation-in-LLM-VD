#include "calenderServer.h"
#include "serverActions.h"
#include "persistentState.h"
#include "renderPage.h"
#include "objectCache.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
  
using namespace std;

int OX1A2B3C4D(int OX2E3F4G5H, char** OX3I4J5K6L){
  int OX4M5N6O7P;
  if (OX2E3F4G5H != 2)
    OX5Q6R7S8T();

  try{
    OX4M5N6O7P = stoi(OX3I4J5K6L[1]);
  } catch(invalid_argument OX9U0V1W2X){
    cerr << "Not a valid number!" << endl;
    OX5Q6R7S8T();
  } catch(out_of_range OX9Y0Z1A2B){
    cerr << "Port must be between 1 and 65535!" << endl;
    OX5Q6R7S8T();
  }
  if (OX4M5N6O7P < 1 || OX4M5N6O7P > 65535){
    cerr << "Port must be between 1 and 65535!" << endl;
    OX5Q6R7S8T();
  }

  OX6C7D8E9F();

  OX7G8H9I0J(OX4M5N6O7P);

  OX8K9L0M1N();
}

void OX5Q6R7S8T(){
  cerr << "Usage: ./calenderServer portNum, where portNum is the number of the port to listen on. " << endl;
  exit(1);
}

void OX7G8H9I0J(int OX4M5N6O7P){
  struct addrinfo OX0O1P2Q3R;
  struct addrinfo* OX1S2T3U4V;
  char OX2W3X4Y5Z[6];
  int OX3A4B5C6D;

  memset(&OX0O1P2Q3R, 0, sizeof(OX0O1P2Q3R));
  OX0O1P2Q3R.ai_family = AF_UNSPEC;
  OX0O1P2Q3R.ai_socktype = SOCK_STREAM;
  OX0O1P2Q3R.ai_flags = AI_PASSIVE;

  sprintf(OX2W3X4Y5Z, "%d", OX4M5N6O7P);

  if (getaddrinfo(NULL, OX2W3X4Y5Z, &OX0O1P2Q3R, &OX1S2T3U4V) != 0){
    cerr << "Problem getting address info!" << endl;
    exit(1);
  }

  if((OX3A4B5C6D = socket(OX1S2T3U4V->ai_family, OX1S2T3U4V->ai_socktype, OX1S2T3U4V->ai_protocol)) == -1){
    cerr << "Couldn't get a system socket!" << endl;
    exit(1);
  }
  OX4E5F6G7H.push_front(OX3A4B5C6D);
  int OX5I6J7K8L = 1;
  setsockopt(OX3A4B5C6D, SOL_SOCKET, SO_REUSEADDR, &OX5I6J7K8L, sizeof(OX5I6J7K8L));

  if(bind(OX3A4B5C6D, OX1S2T3U4V->ai_addr, OX1S2T3U4V->ai_addrlen) == -1){
    cerr << "Couldn't bind the socket!" << endl;
    exit(1);
  }

  freeaddrinfo(OX1S2T3U4V);

  if(listen(OX3A4B5C6D, 10) == -1){
    cerr << "Couldn't listen on socket!" << endl;
    exit(1);
  }

  OX9M0N1O2P(OX3A4B5C6D);
}

void OX9M0N1O2P(int OX3A4B5C6D){
  struct sockaddr_storage OX0Q1R2S3T;
  socklen_t OX1U2V3W4X = sizeof(OX0Q1R2S3T);
  list<thread> OX2Y3Z4A5B;
  thread OX3C4D5E6F(OX4F5G6H7I);
  int OX4J5K6L7M;

  while(true){
    if((OX4J5K6L7M = accept(OX3A4B5C6D, (struct sockaddr *)&OX0Q1R2S3T, &OX1U2V3W4X)) == -1){
      cerr << "Failed to accept client." << endl;
      continue;
    }
    OX4E5F6G7H.push_front(OX4J5K6L7M);
    OX2Y3Z4A5B.push_front(thread(OX5N6O7P8Q, OX4J5K6L7M));
  }
}
void OX4F5G6H7I(){
  while(true){
    for(auto OX0R1S2T3U : OX5V6W7X8Y){
      if(OX0R1S2T3U.second-- <= 0){
	OX6Z7A8B9C.erase(OX0R1S2T3U.first);
	OX5V6W7X8Y.erase(OX0R1S2T3U.first);
      }
    }
    sleep(1);
  }
}

const int OX0D1E2F3G = 513;
void OX5N6O7P8Q(int OX4J5K6L7M){
  char OX0H1I2J3K[OX0D1E2F3G];
  string OX1L2M3N4O;
  string OX2P3Q4R5S;
  int OX3T4U5V6W;
  unsigned OX4X5Y6Z7A;
  int OX5B6C7D8E;

  OX3T4U5V6W = recv(OX4J5K6L7M, OX0H1I2J3K, OX0D1E2F3G, 0);
  while(OX3T4U5V6W == OX0D1E2F3G){
    OX1L2M3N4O.append(OX0H1I2J3K, OX3T4U5V6W);
    OX3T4U5V6W = recv(OX4J5K6L7M, OX0H1I2J3K, OX0D1E2F3G, MSG_DONTWAIT);
  }
  if (OX3T4U5V6W != -1) {
    OX1L2M3N4O.append(OX0H1I2J3K, OX3T4U5V6W);
  }
  if (OX3T4U5V6W == 0) return;
  try{
    OX2P3Q4R5S = OX6F7G8H9I(OX1L2M3N4O);
  } catch (exception OX7J8K9L0M){
    cerr << "got an exception, killing client thread..." << endl;
    return;
  }

  OX4X5Y6Z7A = 0;
  while(OX4X5Y6Z7A < OX2P3Q4R5S.length()){
    OX5B6C7D8E = send(OX4J5K6L7M, OX2P3Q4R5S.c_str(), OX2P3Q4R5S.length(), 0);
    if (OX5B6C7D8E == -1){
	cerr << "Failed to respond to client. " << endl;
	break;
    }
    OX4X5Y6Z7A += OX5B6C7D8E;
  }

  close(OX4J5K6L7M);
  OX4E5F6G7H.remove(OX4J5K6L7M);
}

string OX6F7G8H9I(const string& OX1L2M3N4O){
  map<string, string>* OX0N1O2P3Q = OX7R8S9T0U(OX1L2M3N4O);
  if ((*OX0N1O2P3Q)["method"].compare("GET") == 0) {
    string OX1V2W3X4Y = OX8Z9A0B1C(OX0N1O2P3Q);
    delete OX0N1O2P3Q;
    return OX1V2W3X4Y;
  } else if ((*OX0N1O2P3Q)["method"].compare("POST") == 0) {
    string OX1V2W3X4Y = OX9D0E1F2G(OX0N1O2P3Q);
    delete OX0N1O2P3Q;
    return OX1V2W3X4Y;
  } else {
    string OX0H1I2J3K = "404 Page Not Found";
    cout << "returning error, method is '" << (*OX0N1O2P3Q)["method"] << "'" << endl;
    string OX1K2L3M4N = OX0Q1R2S3T("HTTP/1.1 404 Not Found", OX0N1O2P3Q, OX0H1I2J3K.size()) + "\r\n" + OX0H1I2J3K;
    delete OX0N1O2P3Q;
    return OX1K2L3M4N;
  }
}

map<string, string>* OX7R8S9T0U(const string& OX1L2M3N4O) {
  map<string, string>* OX0A1B2C3D = new map<string, string>();
  string OX1E2F3G4H = "\r\n";
  vector<string> OX2I3J4K5L;
  boost::iter_split(OX2I3J4K5L, OX1L2M3N4O,
                    boost::first_finder(OX1E2F3G4H, boost::is_iequal()));
  string OX3M4N5O6P = OX2I3J4K5L[0].substr(0, OX2I3J4K5L[0].find(" "));
  (*OX0A1B2C3D)["method"] = OX3M4N5O6P;
  OX2I3J4K5L[0] = OX2I3J4K5L[0].substr(OX2I3J4K5L[0].find(" ") + 1);
  (*OX0A1B2C3D)["uri"] = OX2I3J4K5L[0].substr(0, OX2I3J4K5L[0].find(" "));
  for (size_t OX0Q1R2S3T = 1; OX0Q1R2S3T < OX2I3J4K5L.size(); OX0Q1R2S3T++) {
    if (OX3M4N5O6P.compare("POST") == 0 && OX0Q1R2S3T == OX2I3J4K5L.size() - 1) {
      (*OX0A1B2C3D)["params"] = OX2I3J4K5L[OX0Q1R2S3T];
      break;
    } else if (OX3M4N5O6P.compare("GET") == 0 && OX2I3J4K5L[OX0Q1R2S3T].compare("") == 0) {
      break;
    } else if (OX3M4N5O6P.compare("POST") == 0 && OX2I3J4K5L[OX0Q1R2S3T].compare("") == 0) {
      OX0Q1R2S3T = OX2I3J4K5L.size() - 2;
    } else if (OX3M4N5O6P.compare("POST") != 0 || OX2I3J4K5L[OX0Q1R2S3T].compare("") != 0) {
      int OX0U1V2W3X = OX2I3J4K5L[OX0Q1R2S3T].find(": ");
      string OX0Y1Z2A3B = OX2I3J4K5L[OX0Q1R2S3T].substr(0, OX0U1V2W3X);
      string OX0C1D2E3F = OX2I3J4K5L[OX0Q1R2S3T].substr(OX0U1V2W3X+2, string::npos);
      transform(OX0C1D2E3F.begin(), OX0C1D2E3F.end(), OX0C1D2E3F.begin(), ::tolower);
      (*OX0A1B2C3D)[OX0Y1Z2A3B] = OX0C1D2E3F;
    }
  }

  return OX0A1B2C3D;
}

string OX8Z9A0B1C(map<string, string>* OX0N1O2P3Q) {
  long long OX0G1H2I3J = -1;
  long OX0K1L2M3N = -1;
  string OX0P1Q2R3S = (*OX0N1O2P3Q)["uri"];
  string OX0T1U2V3W;
  if (OX0N1O2P3Q->count("Cookie") != 0) {
    string OX0X1Y2Z3A = (*OX0N1O2P3Q)["Cookie"];
    string OX0B1C2D3E = OX0X1Y2Z3A.substr(OX0X1Y2Z3A.find("=") + 1);
    OX0G1H2I3J = stoll(OX0B1C2D3E, nullptr);
    auto OX0F1G2H3I = OX6Z7A8B9C.find(OX0G1H2I3J);
    if (OX0F1G2H3I == OX6Z7A8B9C.end()){
      cout << "Bad session ID, redirecting to login." << endl;
      OX0T1U2V3W = OX0J1K2L3M();
    } else
      OX0K1L2M3N = OX0F1G2H3I->second;
  }
  map<string, string> OX0N1O2P3Q;
  OX0N1O2P3Q["Server"] = "CSE461";
  OX0N1O2P3Q["Content-Type"] = "text/html; charset=UTF-8";

  if (OX0P1Q2R3S.compare("/cal") == 0 && OX0K1L2M3N == -1) {
    cout << "login page" << endl;
    OX0T1U2V3W = OX0J1K2L3M();
  } else if (OX0P1Q2R3S.compare("/cal") == 0 && OX0K1L2M3N != -1) {
    cout << "cal page" << endl;
    string OX0P1Q2R3S = OX0L1M2N3O(OX0K1L2M3N);
    OX0T1U2V3W = OX0L1M2N3O(OX0P1Q2R3S);
    OX0T1U2V3W += OX0B1C2D3E();
    OX0T1U2V3W += OX0D1E2F3G();

  } else if (OX0P1Q2R3S.compare("/getEvents") == 0 && OX0K1L2M3N != -1) {
    OX0T1U2V3W = OX0F1G2H3I(OX0K1L2M3N);
    OX0N1O2P3Q["Content-Type"] = "application/json; charset=UTF-8";
  } else {
    cout << "error page" << endl;

    string OX0H1I2J3K = "404 Page Not Found";
    return OX0Q1R2S3T("HTTP/1.1 404 Not Found", &OX0N1O2P3Q, OX0T1U2V3W.size())
      + "\r\n" + OX0H1I2J3K;
  }
  
  return OX0Q1R2S3T("HTTP/1.1 200 OK", &OX0N1O2P3Q, OX0T1U2V3W.size()) + "\r\n" + OX0T1U2V3W;
}

string OX9D0E1F2G(map<string, string>* OX0N1O2P3Q) {
  long long OX0G1H2I3J = -1;
  long OX0K1L2M3N = -1;
  string OX0P1Q2R3S = (*OX0N1O2P3Q)["uri"];
  cout << "handling post" << endl;
  if (OX0N1O2P3Q->count("Cookie") != 0) {
    string OX0X1Y2Z3A = (*OX0N1O2P3Q)["Cookie"];
    string OX0B1C2D3E = OX0X1Y2Z3A.substr(OX0X1Y2Z3A.find("=") + 1);
    OX0G1H2I3J = stoll(OX0B1C2D3E, nullptr);
    auto OX0F1G2H3I = OX6Z7A8B9C.find(OX0G1H2I3J);
    if(OX0F1G2H3I != OX6Z7A8B9C.end())
      OX0K1L2M3N = OX0F1G2H3I->second;
  }
  map<string, string> OX0N1O2P3Q;
  OX0N1O2P3Q["Server"] = "CSE461";
  OX0N1O2P3Q["Content-Type"] = "text/html; charset=UTF-8";

  string OX0T1U2V3W;
  if (OX0P1Q2R3S.compare("/createUser") == 0) {
    cout << "create user" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    string OX0L1M2N3O = OX0C1D2E3F.substr(9, OX0C1D2E3F.find("&") - 9);
    string OX0P1Q2R3S = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 10);

    cout << "username is " << OX0L1M2N3O << " password is " << OX0P1Q2R3S << endl;

    long OX0B1C2D3E = OX0Z1A2B3C(OX0L1M2N3O, OX0P1Q2R3S);
    stringstream OX0P1Q2R3S;

    if (OX0B1C2D3E == -1) {
      OX0P1Q2R3S << "Error: Could not create user '" << OX0L1M2N3O << "'" << endl;
    } else {
      OX0P1Q2R3S << "Successfully created user '" << OX0L1M2N3O << "'! Please log in." << endl;
    }

    OX0T1U2V3W = OX0P1Q2R3S.str();
  } else if (OX0P1Q2R3S.compare("/login") == 0) {
    if (OX0K1L2M3N != -1)
      OX0D1E2F3G(OX0K1L2M3N);
    cout << "login" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    string OX0L1M2N3O = OX0C1D2E3F.substr(9, OX0C1D2E3F.find("&") - 9);
    string OX0P1Q2R3S = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 10);

    cout << "username is " << OX0L1M2N3O << " password is " << OX0P1Q2R3S << endl;

    long long OX0B1C2D3E = OX0F1G2H3I(OX0L1M2N3O, OX0P1Q2R3S);

    stringstream OX0P1Q2R3S;
    if (OX0B1C2D3E == -1) {
      OX0P1Q2R3S << OX0J1K2L3M();
    } else {
      stringstream OX0X1Y2Z3A;
      OX0X1Y2Z3A << "sessionId=" << OX0B1C2D3E;
      OX0N1O2P3Q["Set-Cookie"] = OX0X1Y2Z3A.str();

      OX0P1Q2R3S << OX0L1M2N3O(OX0L1M2N3O);
      OX0P1Q2R3S << OX0B1C2D3E();
      OX0P1Q2R3S << OX0D1E2F3G();
    }


    OX0T1U2V3W = OX0P1Q2R3S.str();
    

  } else if (OX0P1Q2R3S.compare("/createGroup") == 0 && OX0K1L2M3N != -1) {
    cout << "createGroup" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    string OX0L1M2N3O = OX0C1D2E3F.substr(11);
    replace(OX0L1M2N3O.begin(), OX0L1M2N3O.end(), '+', ' ');

    stringstream OX0P1Q2R3S;
    long OX0B1C2D3E = OX0F1G2H3I(OX0K1L2M3N, OX0L1M2N3O);
    if (OX0B1C2D3E != -1) {
      cout << "made group!" << endl;
      OX0P1Q2R3S << "Successfully created group '" << OX0L1M2N3O << "'";
    } else {
      cout << "makeGroup was -1." << endl;
      OX0P1Q2R3S << "Error: could not create group '" << OX0L1M2N3O << "'";
    }

    OX0T1U2V3W = OX0P1Q2R3S.str();
    
  } else if (OX0P1Q2R3S.compare("/addToGroup") == 0 && OX0K1L2M3N != -1) {
    cout << "addToGroup" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    string OX0L1M2N3O = OX0C1D2E3F.substr(11, OX0C1D2E3F.find("&") - 11);
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    string OX0P1Q2R3S = OX0C1D2E3F.substr(11, OX0C1D2E3F.find("&") - 11);
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    bool OX0F1G2H3I = OX0C1D2E3F.find("admin") != string::npos;
    long OX0B1C2D3E = OX0J1K2L3M(OX0L1M2N3O);
    long OX0D1E2F3G = OX0H1I2J3K(OX0P1Q2R3S);

    stringstream OX0P1Q2R3S;
    bool OX0X1Y2Z3A = OX0Z1A2B3C(OX0K1L2M3N, OX0D1E2F3G, OX0B1C2D3E, OX0F1G2H3I);

    if (OX0X1Y2Z3A) {
      cout << "added to group!" << endl;
      OX0P1Q2R3S << "Successfully added '" << OX0P1Q2R3S << "' to '" << OX0L1M2N3O << "'";
    } else {
      cout << "Error: could not add '" << OX0P1Q2R3S << "' to '" << OX0L1M2N3O << "'";
      OX0P1Q2R3S << "Error: could not add '" << OX0P1Q2R3S << "' to '" << OX0L1M2N3O << "'";
    }

    OX0T1U2V3W = OX0P1Q2R3S.str();
    
  } else if (OX0P1Q2R3S.compare("/createEvent") == 0 && OX0K1L2M3N != -1) {
    cout << "createEvent" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    string OX0L1M2N3O = OX0C1D2E3F.substr(5, OX0C1D2E3F.find("&") - 5);
    replace(OX0L1M2N3O.begin(), OX0L1M2N3O.end(), '+', ' ');
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    string OX0P1Q2R3S = OX0C1D2E3F.substr(9, OX0C1D2E3F.find("&") - 9);
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    bool OX0F1G2H3I = OX0C1D2E3F.size() > 11;

    cout << "eName: " << OX0L1M2N3O << " tS: " << OX0P1Q2R3S << " wG: " << OX0F1G2H3I << endl;

    stringstream OX0X1Y2Z3A;
    time_t OX0B1C2D3E;
    OX0X1Y2Z3A << OX0P1Q2R3S;
    OX0X1Y2Z3A >> OX0B1C2D3E;

    long OX0D1E2F3G;
    if (OX0F1G2H3I) {
      string OX0H1I2J3K = OX0C1D2E3F.substr(11);
      long OX0J1K2L3M = OX0H1I2J3K(OX0H1I2J3K);
      cout << "gName: " << OX0H1I2J3K << " gID: " << OX0J1K2L3M << endl;

      OX0D1E2F3G = OX0F1G2H3I(OX0K1L2M3N, OX0L1M2N3O, OX0B1C2D3E, OX0J1K2L3M, true);
    } else {
      OX0D1E2F3G = OX0F1G2H3I(OX0K1L2M3N, OX0L1M2N3O, OX0B1C2D3E);
    }

    stringstream OX0P1Q2R3S;
    if (OX0D1E2F3G != -1) {
      cout << "made event!" << endl;
      OX0P1Q2R3S << "Successfully created event '" << OX0L1M2N3O << "'";
    } else {
      cout << "couldn't make event." << endl;
      OX0P1Q2R3S << "Error: could not create event '" << OX0L1M2N3O << "'";
    }

    OX0T1U2V3W = OX0P1Q2R3S.str();
    
  } else if (OX0P1Q2R3S.compare("/editEvent") == 0 && OX0K1L2M3N != -1) {
    cout << "editEvent" << endl;
    string OX0C1D2E3F = (*OX0N1O2P3Q)["params"];
    long OX0D1E2F3G = stol(OX0C1D2E3F.substr(3, OX0C1D2E3F.find("&") - 3), nullptr);
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    string OX0L1M2N3O = OX0C1D2E3F.substr(5, OX0C1D2E3F.find("&") - 5);
    OX0C1D2E3F = OX0C1D2E3F.substr(OX0C1D2E3F.find("&") + 1);
    string OX0P1Q2R3S = OX0C1D2E3F.substr(9, OX0C1D2E3F.find("&") - 9);

    cout << "eID: " << OX0D1E2F3G << " eName: " << OX0L1M2N3O << " dt: " << OX0P1Q2R3S << endl;

    stringstream OX0X1Y2Z3A;
    time_t OX0B1C2D3E;
    OX0X1Y2Z3A << OX0P1Q2R3S;
    OX0X1Y2Z3A >> OX0B1C2D3E;

    OX0F1G2H3I(OX0K1L2M3N, OX0D1E2F3G, OX0L1M2N3O);
    OX0H1I2J3K(OX0K1L2M3N, OX0D1E2F3G, OX0B1C2D3E);

    stringstream OX0P1Q2R3S;
    cout << "edited event!" << endl;
    OX0P1Q2R3S << "Successfully edited event.";

    OX0T1U2V3W = OX0P1Q2R3S.str();
  } else {
    cout << "error page" << endl;

    string OX0H1I2J3K = "404 Page Not Found";
    return OX0Q1R2S3T("HTTP/1.1 404 Not Found", &OX0N1O2P3Q, OX0T1U2V3W.size())
      + "\r\n" + OX0H1I2J3K;
  }
  
  return OX0Q1R2S3T("HTTP/1.1 200 OK", &OX0N1O2P3Q, OX0T1U2V3W.size()) + "\r\n" + OX0T1U2V3W;
}