#include "../../auth/proto/auth.pb.h"
#include "../../userstore/proto/userstore.pb.h"
#include "../userhandler.h"

int UserHandler::Update(UpdateRequest &request, UpdateResponse &response) {
  MySvr::Auth::VerifyTicketRequest verifyTicketReq;
  MySvr::Auth::VerifyTicketResponse verifyTicketResp;
  verifyTicketReq.set_user_id(request.user_id());
  verifyTicketReq.set_ticket(request.ticket());
  int ret = Core::MySvrClient().RpcCall(verifyTicketReq, verifyTicketResp);
  if (ret) {
    response.set_message("verifyTicket failed.");
    return ret;
  }
  MySvr::UserStore::UpdateUserRequest updateUserReq;
  MySvr::UserStore::UpdateUserResponse updateUserResp;
  updateUserReq.mutable_user()->set_user_id(request.user_id());
  updateUserReq.mutable_user()->set_nick_name(request.nick_name());
  updateUserReq.mutable_user()->set_password(request.password());
  ret = Core::MySvrClient().RpcCall(updateUserReq, updateUserResp);
  if (ret) {
    response.set_message("updateUser failed.");
    return ret;
  }
  response.set_message("success");
  return 0;
}