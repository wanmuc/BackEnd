#include "../../authstore/proto/authstore.pb.h"
#include "../authhandler.h"

int AuthHandler::GenTicket(GenTicketRequest &request, GenTicketResponse &response) {
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
  setTicketReq.set_expire_time(request.expire_time());
  int ret = Core::MySvrClient().RpcCall(setTicketReq, setTicketResp);
  if (ret) {
    response.set_message("failed");
    return ret;
  }
  response.set_ticket(ticket);
  response.set_message("success");
  return 0;
}