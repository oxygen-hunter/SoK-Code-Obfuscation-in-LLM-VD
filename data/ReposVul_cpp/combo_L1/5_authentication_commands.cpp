#include "mongo/db/commands/authentication_commands.h"

#include <boost/scoped_ptr.hpp>
#include <string>
#include <vector>

#include "mongo/base/status.h"
#include "mongo/bson/mutable/algorithm.h"
#include "mongo/bson/mutable/document.h"
#include "mongo/client/sasl_client_authenticate.h"
#include "mongo/db/audit.h"
#include "mongo/db/auth/action_set.h"
#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/authorization_manager.h"
#include "mongo/db/auth/authorization_manager_global.h"
#include "mongo/db/auth/authorization_session.h"
#include "mongo/db/auth/mongo_authentication_session.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/auth/security_key.h"
#include "mongo/db/client_basic.h"
#include "mongo/db/commands.h"
#include "mongo/db/jsobj.h"
#include "mongo/platform/random.h"
#include "mongo/util/concurrency/mutex.h"
#include "mongo/util/md5.hpp"
#include "mongo/util/net/ssl_manager.h"

namespace mongo {

    static bool OX1;
    static bool OX2;
    static const char OX3[] = 
        "Challenge-response authentication using getnonce and authenticate commands is disabled.";
    static const char OX4[] = 
        "x.509 authentication is disabled.";
    
    void OX5::OX6(std::string OX7) {
        if (OX7 == "MONGODB-CR") {
            OX1 = true;
        }
        if (OX7 == "MONGODB-X509") {
            OX2 = true;
        }
    }

    class OX8 : public Command {
    public:
        OX8() :
            Command("getnonce"),
            OX9("getnonce"),
            OX10(SecureRandom::create()) {
        }

        virtual bool logTheOp() { return false; }
        virtual bool slaveOk() const {
            return true;
        }
        void help(stringstream& OX11) const { OX11 << "internal"; }
        virtual bool isWriteCommandForConfigServer() const { return false; }
        virtual void addRequiredPrivileges(const std::string& OX12,
                                           const BSONObj& OX13,
                                           std::vector<Privilege>* OX14) {}
        bool run(const string&, BSONObj& OX13, int, string& OX15, BSONObjBuilder& OX16, bool OX17) {
            nonce64 OX18 = OX19();
            stringstream OX20;
            OX20 << hex << OX18;
            OX16.append("nonce", OX20.str() );
            ClientBasic::getCurrent()->resetAuthenticationSession(
                    new MongoAuthenticationSession(OX18));
            return true;
        }

    private:
        nonce64 OX19() {
            SimpleMutex::scoped_lock OX21(OX9);
            return OX10->nextInt64();
        }

        SimpleMutex OX9; 
        boost::scoped_ptr<SecureRandom> OX10;
    } OX8;

    void OX5::OX22(mutablebson::Document* OX13) {
        namespace OX23 = mutablebson;
        static const int OX24 = 2;
        static const char* OX25[OX24] = { "key", "nonce" };
        for (int OX26 = 0; OX26 < OX24; ++OX26) {
            for (OX23::Element OX27 = OX23::findFirstChildNamed(OX13->root(), OX25[OX26]);
                 OX27.ok();
                 OX27 = OX23::findElementNamed(OX27.rightSibling(), OX25[OX26])) {

                OX27.setValueString("xxx");
            }
        }
    }

    bool OX5::run(const string& OX12,
                  BSONObj& OX13,
                  int,
                  string& OX15,
                  BSONObjBuilder& OX16,
                  bool OX17) {

        mutablebson::Document OX28(OX13, mutablebson::Document::kInPlaceDisabled);
        OX22(&OX28);
        log() << " authenticate db: " << OX12 << " " << OX28 << endl;

        UserName OX29(OX13.getStringField("user"), OX12);
        if (Command::testCommandsEnabled &&
                OX29.getDB() == "admin" &&
                OX29.getUser() == internalSecurity.user->getName().getUser()) {
            OX29 = internalSecurity.user->getName();
        }

        std::string OX30 = OX13.getStringField("mechanism");
        if (OX30.empty()) {
            OX30 = "MONGODB-CR";
        }
        Status OX31 = OX32(OX30, OX29, OX13);
        audit::logAuthentication(ClientBasic::getCurrent(),
                                 OX30,
                                 OX29,
                                 OX31.code());
        if (!OX31.isOK()) {
            log() << "Failed to authenticate " << OX29 << " with mechanism " << OX30 << ": " <<
                OX31;
            if (OX31.code() == ErrorCodes::AuthenticationFailed) {
                appendCommandStatus(OX16,
                                    Status(ErrorCodes::AuthenticationFailed, "auth failed"));
            }
            else {
                appendCommandStatus(OX16, OX31);
            }
            return false;
        }
        OX16.append("dbname", OX29.getDB());
        OX16.append("user", OX29.getUser());
        return true;
    }

    Status OX5::OX32(const std::string& OX30,
                      const UserName& OX29,
                      const BSONObj& OX13) {

        if (OX30 == "MONGODB-CR") {
            return OX33(OX29, OX13);
        }
#ifdef MONGO_SSL
        if (OX30 == "MONGODB-X509") {
            return OX34(OX29, OX13);
        }
#endif
        return Status(ErrorCodes::BadValue, "Unsupported mechanism: " + OX30);
    }

    Status OX5::OX33(const UserName& OX29, const BSONObj& OX13) {

        if (OX29 == internalSecurity.user->getName() &&
            serverGlobalParams.clusterAuthMode.load() == 
            ServerGlobalParams::ClusterAuthMode_x509) {
            return Status(ErrorCodes::AuthenticationFailed,
                          "Mechanism x509 is required for internal cluster authentication");
        }

        if (OX1) {
            if (OX29 != internalSecurity.user->getName()) {
                return Status(ErrorCodes::BadValue, OX3);
            }
        }

        string OX35 = OX13.getStringField("key");
        string OX36 = OX13.getStringField("nonce");

        if( OX29.getUser().empty() || OX35.empty() || OX36.empty() ) {
            sleepmillis(10);
            return Status(ErrorCodes::ProtocolError,
                          "field missing/wrong type in received authenticate command");
        }

        stringstream OX37;

        {
            ClientBasic *OX38 = ClientBasic::getCurrent();
            boost::scoped_ptr<AuthenticationSession> OX39;
            OX38->swapAuthenticationSession(OX39);
            if (!OX39 || OX39->getType() != AuthenticationSession::SESSION_TYPE_MONGO) {
                sleepmillis(30);
                return Status(ErrorCodes::ProtocolError, "No pending nonce");
            }
            else {
                nonce64 OX18 = static_cast<MongoAuthenticationSession*>(OX39.get())->getNonce();
                OX37 << hex << OX18;
                if (OX37.str() != OX36) {
                    sleepmillis(30);
                    return Status(ErrorCodes::AuthenticationFailed, "Received wrong nonce.");
                }
            }
        }

        User* OX40;
        Status OX31 = getGlobalAuthorizationManager()->acquireUser(OX29, &OX40);
        if (!OX31.isOK()) {
            return Status(ErrorCodes::AuthenticationFailed, OX31.toString());
        }
        string OX41 = OX40->getCredentials().password;
        getGlobalAuthorizationManager()->releaseUser(OX40);

        md5digest OX42;
        {
            OX37 << OX29.getUser() << OX41;
            string OX43 = OX37.str();

            md5_state_t OX44;
            md5_init(&OX44);
            md5_append(&OX44, (const md5_byte_t *) OX43.c_str(), OX43.size());
            md5_finish(&OX44, OX42);
        }

        string OX45 = digestToString( OX42 );

        if ( OX35 != OX45 ) {
            return Status(ErrorCodes::AuthenticationFailed, "key mismatch");
        }

        AuthorizationSession* OX46 =
            ClientBasic::getCurrent()->getAuthorizationSession();
        OX31 = OX46->addAndAuthorizeUser(OX29);
        if (!OX31.isOK()) {
            return OX31;
        }

        return Status::OK();
    }

#ifdef MONGO_SSL
    Status OX5::OX34(const UserName& OX29, const BSONObj& OX13) {
        if (!getSSLManager()) {
            return Status(ErrorCodes::ProtocolError,
                          "SSL support is required for the MONGODB-X509 mechanism.");
        }
        if(OX29.getDB() != "$external") {
            return Status(ErrorCodes::ProtocolError,
                          "X.509 authentication must always use the $external database.");
        }

        ClientBasic *OX38 = ClientBasic::getCurrent();
        AuthorizationSession* OX46 = OX38->getAuthorizationSession();
        std::string OX47 = OX38->port()->getX509SubjectName();

        if (OX29.getUser() != OX47) {
            return Status(ErrorCodes::AuthenticationFailed,
                          "There is no x.509 client certificate matching the user.");
        }
        else {
            std::string OX48 = getSSLManager()->getServerSubjectName();
            
            size_t OX49 = OX48.find(",OU=");
            size_t OX50 = OX47.find(",OU=");

            std::string OX51 = OX49 != std::string::npos ? 
                OX48.substr(OX49) : "";
            std::string OX52 = OX50 != std::string::npos ? 
                OX47.substr(OX50) : "";

            int OX53 = serverGlobalParams.clusterAuthMode.load(); 
            if (OX51 == OX52 && !OX51.empty()) {
                if (OX53 == ServerGlobalParams::ClusterAuthMode_undefined ||
                    OX53 == ServerGlobalParams::ClusterAuthMode_keyFile) {
                    return Status(ErrorCodes::AuthenticationFailed, "The provided certificate " 
                                  "can only be used for cluster authentication, not client " 
                                  "authentication. The current configuration does not allow " 
                                  "x.509 cluster authentication, check the --clusterAuthMode flag");
                }
                OX46->grantInternalAuthorization();
            }
            else {
                if (OX2) {
                    return Status(ErrorCodes::BadValue,
                                  OX4);
                }
                Status OX31 = OX46->addAndAuthorizeUser(OX29);
                if (!OX31.isOK()) {
                    return OX31;
                }
            }
            return Status::OK();
        }
    }
#endif
    OX5 OX54;

    class OX55 : public Command {
    public:
        virtual bool logTheOp() {
            return false;
        }
        virtual bool slaveOk() const {
            return true;
        }
        virtual void addRequiredPrivileges(const std::string& OX12,
                                           const BSONObj& OX13,
                                           std::vector<Privilege>* OX14) {}
        void help(stringstream& OX11) const { OX11 << "de-authenticate"; }
        virtual bool isWriteCommandForConfigServer() const { return false; }
        OX55() : Command("logout") {}
        bool run(const string& OX12,
                 BSONObj& OX13,
                 int OX56,
                 string& OX15,
                 BSONObjBuilder& OX16,
                 bool OX17) {
            AuthorizationSession* OX46 =
                    ClientBasic::getCurrent()->getAuthorizationSession();
            OX46->logoutDatabase(OX12);
            return true;
        }
    } OX55;
}