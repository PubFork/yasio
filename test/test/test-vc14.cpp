#include "xxsocket.h"
#include "async_tcp_client.h"
#include "pcode_autog_client_messages.h"
#include "http_client.hpp"

#if defined(_WIN32)
#define sleep(sec) Sleep(sec * 1000)
#define msleep(msec) Sleep(msec)
#endif

using namespace purelib::inet;

//////////////////////////////////////////////
// async_tcp_client: ҵ���޹صĻ���tcp�Ķ�����Э�����
// ����Ϊ����ӿ�,��չ���÷�, �����װ��һ��ҵ����,����NetworManager
#define PACKET_HEADER_LENGTH sizeof(messages::MsgHeader)
#define PACKET_MAGIC_NUMBER 0x5a5a

// ��Ϣ��ͷ, ����ʵ��, �ֶο�ѡ,������Ӧ���б�ʾЭ������ȵ��ֶ�
obinarystream pcode_autog_begin_encode(uint16_t command_id)
{
    messages::MsgHeader hdr;
    hdr.command_id = command_id;
    hdr.length = 0;
    hdr.reserved = 0x5a5a;
    hdr.reserved2 = microtime();
    hdr.version = 1;
    return hdr.encode();
}

#if 1
// ������Ϣ����
bool decode_pdu_length(const char* data, size_t datalen, int& len)
{
    if (datalen >= PACKET_HEADER_LENGTH) {

        char hdrs[PACKET_HEADER_LENGTH];

        messages::MsgHeader hdr;
        hdr.decode(hdrs, sizeof(hdrs));

        len = hdr.length + PACKET_HEADER_LENGTH;
        return true;// PACKET_MAGIC_NUMBER == hdr.reserved;
    }
    else {
        len = -1;
        return true;
    }
}

// ������ӳ���,���ͳ�ʱ,���챾�ػػ������
std::vector<char> build_error_msg(int errorcode, const char* errormsg)
{
    messages::LocalErrorResp error;
    error.error_code = errorcode;
    error.error_msg = errormsg;
    auto msg = error.encode();
    return msg.take_buffer();
}

// �յ�������һ����Ϣ��, �����ڱ����
void on_recv_msg(std::vector<char>&& data)
{
    messages::MsgHeader hdr;
    int offset = hdr.decode(data.data(), static_cast<int>(data.size()));
    auto msg = messages::temp_create_message(hdr.command_id);
    switch (msg->get_id())
    { //
    case CID_LOCAL_ERROR_INFO:
        printf("�������,��Ҫ����!\n");
        break;
    case CID_LOGIN_RESP: {
        auto detail = dynamic_cast<messages::LoginResp*>(msg);
        printf("��½%s\n", detail->succeed ? "�ɹ�" : "ʧ��");
        tcpcli->close(); // �ر�����

        if (detail->succeed) {
            // �л�����Ϸ������
            printf("����������Ϸ������...\n");
            tcpcli->set_endpoint("192.168.199.246", "", 20001);
            tcpcli->notify_connect();
        }
        ; }
                         break;
    }
}

void test_tcp_service()
{
    tcpcli->set_connect_listener([](bool succeed, int error) {
        if (succeed)
        { // ���ӳɹ�, ��ɷ����½Э��,����
#if 0
            messages::LoginReq req;
            req.username = "helloworld";
            req.password = "helloworld";
            auto obs = req.encode();
            tcpcli->async_send(obs.move(), [](ErrorCode error) {
                if (error == 0) { // ���ͳɹ�

                }
                else { // ���ܷ��ͳ�ʱ, �����������

                }
            });
#endif
            auto timer = std::shared_ptr<deadline_timer>(new deadline_timer());
            timer->expires_from_now(std::chrono::seconds(3), true);
            timer->async_wait([](bool cancelled) {
                if (!cancelled) {
                    printf("ѭ����ʱ��,ÿ3�봥��һ��.\n");
#if 1
                    std::vector<char> msg;
                    msg.resize(sizeof("hello world\r\n"));
                    memcpy(&msg.front(), "hello world\r\n", sizeof("hello world\r\n") - 1);
                    tcpcli->async_send(std::move(msg));
#endif
                }
            });
        }
    });
    tcpcli->set_callbacks(decode_pdu_length, build_error_msg, on_recv_msg, [](const vdcallback_t& callb) {
        callb();
    });
    tcpcli->set_endpoint("127.0.0.1", "", 80); // ������Ϊ��½������
    tcpcli->start_service();
    auto start = std::chrono::steady_clock::now();
    printf("3���ʼ���ӷ�����...\n");
    bool notify = false;
    while (1) {

        // bool received = tcpcli->collect_received_pdu();
        //if (!received) {
        msleep(1); // ģ��cocos2d-x���߳�
    //}

        if (!notify)
        {
            tcpcli->notify_connect();
            notify = true;
        }
    }
}

void test_https_connect()
{
    xxsocket client;
    // auto epv6 = xxsocket::resolve("www.baidu.com", 443);
    int n = client.xpconnect_n("127.0.0.1", 6001, 3);
    auto flag = client.getipsv();
    printf("ip protocol support flag is:%d\n", flag);
    if (n == 0)
    {
        printf("connect https server success, [%s] --> [%s]", client.local_endpoint().to_string().c_str(), client.peer_endpoint().to_string().c_str());
    }
    else {
        printf("connect failed, code:%d,info:%s", xxsocket::get_last_errno(), xxsocket::get_error_msg(xxsocket::get_last_errno()));
    }
}

int main(int, char**)
{
    // std::string respData = send_http_req("http://x-studio365.com");
    xxsocket client;
    client.open();

    bool check = client.is_nonblocking();

    extern void http_sendemail(const std::string& mailto, const std::string& subject, std::string&& message);

    // 163 mail always response: 554 DT:SPM 163 smtp14,EsCowACH_vF0S7VXPsvDCg--.19895S2 1471499125,...
    int smtp_sendmail(const char * smtpServer, int port, const char* username, const char * password, const char* from, const char * to, const char * subject, const char * body);
  
    // could send 5 mails per hour
    http_sendemail("88888888@qq.com", "ckkjgj", "gfjjihuijwiofoiw\ngfkjkghd whitespace new 20160818 12:08,");

    // test_https_connect();
    test_tcp_service();

    getchar();

    // xxsocket will close socket automatically.

    return 0;
}

#endif