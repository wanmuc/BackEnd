#include "../../authstore/proto/authstore.pb.h"
#include "../authhandler.h"

int AuthHandler::UpdateTicket(UpdateTicketRequest &request, UpdateTicketResponse &response) {
  MySvr::AuthStore::GetTicketRequest getTicketReq;
  MySvr::AuthStore::GetTicketResponse getTicketResp;
  getTicketReq.set_user_id(request.user_id());
  int ret = Core::MySvrClient().RpcCall(getTicketReq, getTicketResp);
  if (ret) {
    response.set_message("failed");
    return ret;
  }
  if (getTicketResp.ticket().ticket() != request.ticket()) {
    response.set_message("ticket is invalid");
    return -2;
  }

  MySvr::AuthStore::SetTicketRequest setTicketReq;
  MySvr::AuthStore::SetTicketResponse setTicketResp;
  setTicketReq.mutable_ticket()->set_user_id(request.user_id());
  std::string ticket;
  for (int i = 0; i < 6; i++) {
    char temp[10]{0};
    sprintf(temp, "%02X", rand());
    ticket += temp;
  }
  setTicketReq.mutable_ticket()->set_ticket(ticket);
  setTicketReq.set_expire_time(36000);
  ret = Core::MySvrClient().RpcCall(setTicketReq, setTicketResp);
  if (ret) {
    response.set_message("failed");
    return ret;
  }
  response.set_ticket(ticket);
  response.set_message("success");
  return 0;
}