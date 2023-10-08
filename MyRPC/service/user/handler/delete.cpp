#include "../../../common/argv.hpp"
#include "../../../core/waitgroup.hpp"
#include "../../auth/proto/auth.pb.h"
#include "../../userstore/proto/userstore.pb.h"
#include "../userhandler.h"

void getUser(void *arg) {
  Common::Argv *getUserArgv = (Common::Argv *)arg;
  MySvr::UserStore::ReadUserRequest readUserReq;
  MySvr::UserStore::ReadUserResponse readUserResp;
  readUserReq.set_user_id(getUserArgv->Arg<std::string>("user_id"));
  int ret = Core::MySvrClient().RpcCall(readUserReq, readUserResp);
  if (ret) {
    getUserArgv->Arg<int>("getUserResult") = -1;
    return;
  }
  getUserArgv->Arg<int>("getUserResult") = 0;
}

void verifyTicket(void *arg) {
  Common::Argv *verifyTicketArgv = (Common::Argv *)arg;
  MySvr::Auth::VerifyTicketRequest verifyTicketReq;
  MySvr::Auth::VerifyTicketResponse verifyTicketResp;
  verifyTicketReq.set_user_id(verifyTicketArgv->Arg<std::string>("user_id"));
  verifyTicketReq.set_ticket(verifyTicketArgv->Arg<std::string>("ticket"));
  int ret = Core::MySvrClient().RpcCall(verifyTicketReq, verifyTicketResp);
  if (ret) {
    verifyTicketArgv->Arg<int>("verifyTicketResult") = -1;
    return;
  }
  verifyTicketArgv->Arg<int>("verifyTicketResult") = 0;
}

int UserHandler::Delete(DeleteRequest &request, DeleteResponse &response) {
  Common::Argv getUserArgv;
  Common::Argv verifyTicketArgv;
  int getUserResult = 0;
  int verifyTicketResult = 0;
  getUserArgv.Set("user_id", request.mutable_user_id());
  getUserArgv.Set("getUserResult", &getUserResult);
  verifyTicketArgv.Set("ticket", request.mutable_ticket());
  verifyTicketArgv.Set("user_id", request.mutable_user_id());
  verifyTicketArgv.Set("verifyTicketResult", &verifyTicketResult);
  Core::WaitGroup waitGroup;
  waitGroup.Add(getUser, &getUserArgv);
  waitGroup.Add(verifyTicket, &verifyTicketArgv);
  waitGroup.Wait();
  TRACE("getUserResult[%d],verifyTicketResult[%d]", getUserResult, verifyTicketResult);
  if (getUserResult) {
    response.set_message("getUser failed.");
    return getUserResult;
  }
  if (verifyTicketResult) {
    response.set_message("verifyTicket failed.");
    return verifyTicketResult;
  }
  MySvr::UserStore::DeleteUserRequest deleteUserReq;
  MySvr::UserStore::DeleteUserResponse deleteUserResp;
  deleteUserReq.set_user_id(request.user_id());
  int ret = Core::MySvrClient().RpcCall(deleteUserReq, deleteUserResp);
  if (ret) {
    response.set_message("delete user failed");
    return ret;
  }
  response.set_message("success");
  return 0;
}