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

    static bool _isCRAuthDisabled;
    static bool _isX509AuthDisabled;
    static const char* getNonceAuthDisabledMessage() {
        return "Challenge-response authentication using getnonce and authenticate commands is disabled.";
    }
    static const char* getX509AuthDisabledMessage() {
        return "x.509 authentication is disabled.";
    }

    void CmdAuthenticate::disableAuthMechanism(std::string authMechanism) {
        if (authMechanism == "MONGODB-CR") {
            _isCRAuthDisabled = true;
        }
        if (authMechanism == "MONGODB-X509") {
            _isX509AuthDisabled = true;
        }
    }

    class CmdGetNonce : public Command {
    public:
        CmdGetNonce() :
            Command("getnonce"),
            _randMutex("getnonce"),
            _random(SecureRandom::create()) {
        }

        virtual bool logTheOp() { return false; }
        virtual bool slaveOk() const {
            return true;
        }
        void help(stringstream& h) const { h << "internal"; }
        virtual bool isWriteCommandForConfigServer() const { return false; }
        virtual void addRequiredPrivileges(const std::string& dbname,
                                           const BSONObj& cmdObj,
                                           std::vector<Privilege>* out) {} // No auth required
        bool run(const string&, BSONObj& cmdObj, int, string& errmsg, BSONObjBuilder& result, bool fromRepl) {
            nonce64 n = getNextNonce();
            stringstream ss;
            ss << hex << n;
            result.append("nonce", ss.str() );
            ClientBasic::getCurrent()->resetAuthenticationSession(
                    new MongoAuthenticationSession(n));
            return true;
        }

    private:
        nonce64 getNextNonce() {
            SimpleMutex::scoped_lock lk(_randMutex);
            return _random->nextInt64();
        }

        SimpleMutex _randMutex;  // Synchronizes accesses to _random.
        boost::scoped_ptr<SecureRandom> _random;
    } cmdGetNonce;

    void CmdAuthenticate::redactForLogging(mutablebson::Document* cmdObj) {
        namespace mmb = mutablebson;
        static const int numRedactedFields = 2;
        static const char* redactedFields[numRedactedFields] = { "key", "nonce" };
        for (int i = 0; i < numRedactedFields; ++i) {
            for (mmb::Element element = mmb::findFirstChildNamed(cmdObj->root(), redactedFields[i]);
                 element.ok();
                 element = mmb::findElementNamed(element.rightSibling(), redactedFields[i])) {

                element.setValueString("xxx");
            }
        }
    }

    bool CmdAuthenticate::run(const string& dbname,
                              BSONObj& cmdObj,
                              int,
                              string& errmsg,
                              BSONObjBuilder& result,
                              bool fromRepl) {

        mutablebson::Document cmdToLog(cmdObj, mutablebson::Document::kInPlaceDisabled);
        redactForLogging(&cmdToLog);
        log() << " authenticate db: " << dbname << " " << cmdToLog << endl;

        UserName user(cmdObj.getStringField("user"), dbname);
        if (Command::testCommandsEnabled &&
                user.getDB() == "admin" &&
                user.getUser() == internalSecurity.user->getName().getUser()) {
            user = internalSecurity.user->getName();
        }

        std::string mechanism = cmdObj.getStringField("mechanism");
        if (mechanism.empty()) {
            mechanism = "MONGODB-CR";
        }
        Status status = _authenticate(mechanism, user, cmdObj);
        audit::logAuthentication(ClientBasic::getCurrent(),
                                 mechanism,
                                 user,
                                 status.code());
        if (!status.isOK()) {
            log() << "Failed to authenticate " << user << " with mechanism " << mechanism << ": " <<
                status;
            if (status.code() == ErrorCodes::AuthenticationFailed) {
                appendCommandStatus(result,
                                    Status(ErrorCodes::AuthenticationFailed, "auth failed"));
            }
            else {
                appendCommandStatus(result, status);
            }
            return false;
        }
        result.append("dbname", user.getDB());
        result.append("user", user.getUser());
        return true;
    }

    Status CmdAuthenticate::_authenticate(const std::string& mechanism,
                                          const UserName& user,
                                          const BSONObj& cmdObj) {

        if (mechanism == "MONGODB-CR") {
            return _authenticateCR(user, cmdObj);
        }
#ifdef MONGO_SSL
        if (mechanism == "MONGODB-X509") {
            return _authenticateX509(user, cmdObj);
        }
#endif
        return Status(ErrorCodes::BadValue, "Unsupported mechanism: " + mechanism);
    }

    Status CmdAuthenticate::_authenticateCR(const UserName& user, const BSONObj& cmdObj) {

        if (user == internalSecurity.user->getName() &&
            serverGlobalParams.clusterAuthMode.load() == 
            ServerGlobalParams::ClusterAuthMode_x509) {
            return Status(ErrorCodes::AuthenticationFailed,
                          "Mechanism x509 is required for internal cluster authentication");
        }

        if (_isCRAuthDisabled) {
            if (user != internalSecurity.user->getName()) {
                return Status(ErrorCodes::BadValue, getNonceAuthDisabledMessage());
            }
        }

        string key = cmdObj.getStringField("key");
        string received_nonce = cmdObj.getStringField("nonce");

        if( user.getUser().empty() || key.empty() || received_nonce.empty() ) {
            sleepmillis(10);
            return Status(ErrorCodes::ProtocolError,
                          "field missing/wrong type in received authenticate command");
        }

        stringstream digestBuilder;

        {
            ClientBasic *client = ClientBasic::getCurrent();
            boost::scoped_ptr<AuthenticationSession> session;
            client->swapAuthenticationSession(session);
            if (!session || session->getType() != AuthenticationSession::SESSION_TYPE_MONGO) {
                sleepmillis(30);
                return Status(ErrorCodes::ProtocolError, "No pending nonce");
            }
            else {
                nonce64 nonce = static_cast<MongoAuthenticationSession*>(session.get())->getNonce();
                digestBuilder << hex << nonce;
                if (digestBuilder.str() != received_nonce) {
                    sleepmillis(30);
                    return Status(ErrorCodes::AuthenticationFailed, "Received wrong nonce.");
                }
            }
        }

        User* userObj;
        Status status = getGlobalAuthorizationManager()->acquireUser(user, &userObj);
        if (!status.isOK()) {
            return Status(ErrorCodes::AuthenticationFailed, status.toString());
        }
        string pwd = userObj->getCredentials().password;
        getGlobalAuthorizationManager()->releaseUser(userObj);

        md5digest d;
        {
            digestBuilder << user.getUser() << pwd;
            string done = digestBuilder.str();

            md5_state_t st;
            md5_init(&st);
            md5_append(&st, (const md5_byte_t *) done.c_str(), done.size());
            md5_finish(&st, d);
        }

        string computed = digestToString( d );

        if ( key != computed ) {
            return Status(ErrorCodes::AuthenticationFailed, "key mismatch");
        }

        AuthorizationSession* authorizationSession =
            ClientBasic::getCurrent()->getAuthorizationSession();
        status = authorizationSession->addAndAuthorizeUser(user);
        if (!status.isOK()) {
            return status;
        }

        return Status::OK();
    }

#ifdef MONGO_SSL
    Status CmdAuthenticate::_authenticateX509(const UserName& user, const BSONObj& cmdObj) {
        if (!getSSLManager()) {
            return Status(ErrorCodes::ProtocolError,
                          "SSL support is required for the MONGODB-X509 mechanism.");
        }
        if(user.getDB() != "$external") {
            return Status(ErrorCodes::ProtocolError,
                          "X.509 authentication must always use the $external database.");
        }

        ClientBasic *client = ClientBasic::getCurrent();
        AuthorizationSession* authorizationSession = client->getAuthorizationSession();
        std::string subjectName = client->port()->getX509SubjectName();

        if (user.getUser() != subjectName) {
            return Status(ErrorCodes::AuthenticationFailed,
                          "There is no x.509 client certificate matching the user.");
        }
        else {
            std::string srvSubjectName = getSSLManager()->getServerSubjectName();
            
            size_t srvClusterIdPos = srvSubjectName.find(",OU=");
            size_t peerClusterIdPos = subjectName.find(",OU=");

            std::string srvClusterId = srvClusterIdPos != std::string::npos ? 
                srvSubjectName.substr(srvClusterIdPos) : "";
            std::string peerClusterId = peerClusterIdPos != std::string::npos ? 
                subjectName.substr(peerClusterIdPos) : "";

            int clusterAuthMode = serverGlobalParams.clusterAuthMode.load(); 
            if (srvClusterId == peerClusterId && !srvClusterId.empty()) {
                if (clusterAuthMode == ServerGlobalParams::ClusterAuthMode_undefined ||
                    clusterAuthMode == ServerGlobalParams::ClusterAuthMode_keyFile) {
                    return Status(ErrorCodes::AuthenticationFailed, "The provided certificate " 
                                  "can only be used for cluster authentication, not client " 
                                  "authentication. The current configuration does not allow " 
                                  "x.509 cluster authentication, check the --clusterAuthMode flag");
                }
                authorizationSession->grantInternalAuthorization();
            }
            else {
                if (_isX509AuthDisabled) {
                    return Status(ErrorCodes::BadValue,
                                  getX509AuthDisabledMessage());
                }
                Status status = authorizationSession->addAndAuthorizeUser(user);
                if (!status.isOK()) {
                    return status;
                }
            }
            return Status::OK();
        }
    }
#endif
    CmdAuthenticate cmdAuthenticate;

    class CmdLogout : public Command {
    public:
        virtual bool logTheOp() {
            return false;
        }
        virtual bool slaveOk() const {
            return true;
        }
        virtual void addRequiredPrivileges(const std::string& dbname,
                                           const BSONObj& cmdObj,
                                           std::vector<Privilege>* out) {} // No auth required
        void help(stringstream& h) const { h << "de-authenticate"; }
        virtual bool isWriteCommandForConfigServer() const { return false; }
        CmdLogout() : Command("logout") {}
        bool run(const string& dbname,
                 BSONObj& cmdObj,
                 int options,
                 string& errmsg,
                 BSONObjBuilder& result,
                 bool fromRepl) {
            AuthorizationSession* authSession =
                    ClientBasic::getCurrent()->getAuthorizationSession();
            authSession->logoutDatabase(dbname);
            return true;
        }
    } cmdLogout;
}