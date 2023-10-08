#include "../../authstore/proto/authstore.pb.h"
#include "../authhandler.h"

int AuthHandler::VerifyTicket(VerifyTicketRequest &request, VerifyTicketResponse &response) {
  MySvr::AuthStore::GetTicketRequest getTicketReq;
  MySvr::AuthStore::GetTicketResponse getTicketResp;
  getTicketReq.set_user_id(request.user_id());
  int ret = Core::MySvrClient().RpcCall(getTicketReq, getTicketResp);
  if (ret) {
    response.set_message("get ticket failed");
    return ret;
  }
  if (request.ticket() != getTicketResp.ticket().ticket()) {
    response.set_message("ticket is invalid");
    return -1;
  }
  response.set_message("success");
  return 0;
}