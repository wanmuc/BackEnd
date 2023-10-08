#include "../../auth/proto/auth.pb.h"
#include "../../userstore/proto/userstore.pb.h"
#include "../userhandler.h"

int UserHandler::Create(CreateRequest &request, CreateResponse &response) {
  MySvr::UserStore::CreateUserRequest createUserReq;
  MySvr::UserStore::CreateUserResponse createUserResp;
  createUserReq.mutable_user()->set_nick_name(request.nick_name());
  createUserReq.mutable_user()->set_password(request.password());
  int ret = Core::MySvrClient().RpcCall(createUserReq, createUserResp);
  if (ret) {
    response.set_message(createUserResp.message());
    return ret;
  }
  MySvr::Auth::GenTicketRequest genTicketReq;
  MySvr::Auth::GenTicketResponse genTicketResp;
  genTicketReq.set_expire_time(3600 * 24);
  genTicketReq.set_user_id(createUserResp.user_id());
  ret = Core::MySvrClient().RpcCall(genTicketReq, genTicketResp);
  if (ret) {
    response.set_message(genTicketResp.message());
    return ret;
  }
  response.set_user_id(createUserResp.user_id());
  response.set_ticket(genTicketResp.ticket());
  response.set_message("success");
  return 0;
}
