#include "../../auth/proto/auth.pb.h"
#include "../../userstore/proto/userstore.pb.h"
#include "../userhandler.h"

int UserHandler::Read(ReadRequest &request, ReadResponse &response) {
  MySvr::Auth::VerifyTicketRequest verifyTicketReq;
  MySvr::Auth::VerifyTicketResponse verifyTicketResp;
  verifyTicketReq.set_user_id(request.user_id());
  verifyTicketReq.set_ticket(request.ticket());
  int ret = Core::MySvrClient().RpcCall(verifyTicketReq, verifyTicketResp);
  if (ret) {
    response.set_message("verifyTicket failed.");
    return ret;
  }
  MySvr::UserStore::ReadUserRequest readUserReq;
  MySvr::UserStore::ReadUserResponse readUserResp;
  readUserReq.set_user_id(request.user_id());
  ret = Core::MySvrClient().RpcCall(readUserReq, readUserResp);
  if (ret) {
    response.set_message("readUser failed.");
    return ret;
  }
  response.set_nick_name(readUserResp.user().nick_name());
  response.set_message("success");
  return 0;
}