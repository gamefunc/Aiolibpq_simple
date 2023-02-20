/* GNU General Public License v3.0

Aiolibpq simple version

author: gamefunc
    website: https://www.gamefunc.top:9029
    github: https://github.com/gamefunc
    qq: 32686647
    weixin: gamefunc
    mail: fevefun@hotmail.com

free to use and modify, but need keep the above information.

test env:
    boost version >= 1.78
    win_10: cl: 19.34.31937;
    debian_11: gcc: 10.2.1(include.replace("std::format", "fmt::format"));  
*/
#include <Aiolibpq_simple.hpp>

Aiolibpq::Aiolibpq(std::string conn_params){
    conn = PQgetSetupConn(conn_params.data());
}// Aiolibpq() 1;

Aiolibpq::Aiolibpq(std::string dbname, std::string user,
        std::string password, std::string host,
        uint16_t port, std::string extra_params){
    std::string conn_params = std::format(
        "dbname={} user={} password={} host={} port={} {}",
        dbname, user, password, host, port, extra_params
    );
    conn = PQgetSetupConn(conn_params.data());
}// Aiolibpq() 2;

Aiolibpq::~Aiolibpq(){ close_conn(); }


Aiolibpq::Aiolibpq(Aiolibpq &&src){
    this->conn = src.conn;
    src.conn = nullptr;
}// Aiolibpq(Aiolibpq &&src)


void Aiolibpq::close_conn(){
    if(conn != nullptr){
        PQfinish(conn);
        conn = nullptr;
    }
}// close_conn()


awaitable<int> Aiolibpq::connect(){
    auto loop = co_await this_coro::executor;
    if(conn == nullptr){ co_return -1; }

    const char* host_name = nullptr;
    while((host_name = PQgetUnresolvHost(conn)) != nullptr){
        tcp::resolver resolver(loop);
        tcp::resolver::iterator ep_iter = 
            co_await resolver.async_resolve(
                host_name, "", use_awaitable);
        tcp::endpoint ep = *ep_iter;
        PQSetUnresolvHost(
            conn, host_name,
            ep.address().to_string().data(),
            ep.address().to_string().size());
    }// while()

    int ok = PQconnectDBStart(conn);
    if(ok == 0){ co_return -2; }
    auto conn_status = PQstatus(conn);
    if(conn_status != CONNECTION_STARTED){
        co_return -3;
    }

    // https://www.gamefunc.top:9029/public/knowledge/postgresql_x_cn/libpq-connect.html
    auto fd = PQsocket(conn);
    auto t = PGRES_POLLING_WRITING;
    while((t == PGRES_POLLING_WRITING) 
            || (t == PGRES_POLLING_READING)){
        co_await asyncio_wait_fd_rw(fd, 
            (t == PGRES_POLLING_WRITING)
                ? tcp::socket::wait_write
                : tcp::socket::wait_read );
        fd = PQsocket(conn);
        t = PQconnectPoll(conn);
    }// while need wait_read || wait_write

    co_return (PQstatus(conn) == CONNECTION_OK) ? 0 : -4;
}// connect()


awaitable<int> Aiolibpq::execute(std::string_view cmd){
    auto loop = co_await this_coro::executor;
    auto fd = PQsocket(conn);
    int ok = 0;
    if(PQstatus(conn) != CONNECTION_OK){
        co_return -1;
    }

    // https://www.gamefunc.top:9029/public/knowledge/postgresql_x_cn/libpq-async.html
    ok = PQsendQuery(conn, cmd.data());
    if(ok == 0){ co_return -2; }

    // conn->status == CONNECTION_BAD will return -1:
    while(ok = PQflush(conn)){
        if(ok == -1){ co_return -3; }
        co_await asyncio_wait_fd_rw(fd, tcp::socket::wait_write);
    }// while PQflush(conn)

    co_return 0;
}// execute()



awaitable<std::tuple<
    std::vector<std::string>,
    std::unordered_map<std::string, std::vector<std::string>> 
        > > Aiolibpq::fetchall(){
    auto loop = co_await this_coro::executor;
    auto fd = PQsocket(conn);
    std::unordered_map<
        std::string, std::vector<std::string>
        > rows_dict;
    std::vector<std::string> order_key_list;
    
    while(PQisBusy(conn)){
        co_await asyncio_wait_fd_rw(
            fd, tcp::socket::wait_read);
        if(PQconsumeInput(conn)){
            while(!PQisBusy(conn)){
                PGresult *rows = PQgetResult(conn);
                auto result_status = PQresultStatus(rows);
                if(rows){
                    if(result_status == PGRES_TUPLES_OK 
                            || result_status == PGRES_TUPLES_OK){
                        if(rows_dict.size() == 0){
                            int err = setup_result_rows_dict_key(
                                rows_dict, order_key_list, rows);
                            if(rows_dict.size() == 0){ continue; }
                        }// if rows_dict not setup;

                        for(int row_num = 0; row_num < PQntuples(rows); row_num++){
                            for(int col_num = 0; col_num < PQnfields(rows); col_num++){
                                rows_dict[order_key_list[col_num]].emplace_back(
                                    PQgetvalue(rows, row_num, col_num)
                                );
                            }// for col_num in range
                        }// for row_num in range:
                    }else{
                        // other status;
                        std::cout << std::format(
                            "fetchall(): ExecStatusType_status_num: {};"
                                " msg: {};\n",
                            (int)result_status,
                            PQresultErrorMessage(rows));
                    }// if else if else status;
                    PQclear(rows);
                }else{
                    // nullptr is recv final;
                    break;
                }// if(rows_p) else;
            }// while(!PQisBusy(conn))
        }// if(PQconsumeInput(conn))
    }// while busy:

    co_return std::make_tuple(order_key_list, rows_dict);
}// fetchall()


int Aiolibpq::setup_result_rows_dict_key(
        std::unordered_map<std::string, std::vector<std::string>> &rows_dict,
        std::vector<std::string> &order_key_list,
        PGresult *rows){
    if(rows_dict.size() > 0){ return -1; }
    if(rows == nullptr){ return -2; }
    // 获取一共几列:
    int num_of_col = PQnfields(rows);
    if(num_of_col == 0){ return -3; }
    order_key_list.clear();

    for(int i = 0; i < num_of_col; i++){
        char *row_name = PQfname(rows, i);
        if(row_name == nullptr){ continue; }
        order_key_list.emplace_back(row_name);
        rows_dict[row_name] = {};
    }// for i in range(num_of_col)
    return 0;
}// setup_result_rows_dict_key()


// /* @brief not yet support co_yield coro task:
//    want:
//     for(auto p: co_await fetchone()){ if(!p){break;} dodo(p); }
// @return PGresult* */
// coro::gen<PGresult*> Aiolibpq::fetchone(){
//     // https://www.gamefunc.top:9029/public/knowledge/postgresql_x_cn/libpq-single-row-mode.html
//     int ok = PQsetSingleRowMode(conn);
//     auto fd = PQsocket(conn);

//     while(PQisBusy(conn)){
//         co_await asyncio_wait_fd_rw(fd, tcp::socket::wait_read);
//         if(PQconsumeInput(conn)){
//             while(!PQisBusy(conn)){
//                 PGresult *one_row = PQgetResult(conn);
//                 co_yield one_row;
//             }// while(!PQisBusy(conn))
//         }// if(PQconsumeInput(conn))
//     }// while(PQisBusy(conn))
// }// fetchone()


awaitable<void> Aiolibpq::asyncio_wait_fd_rw(
        auto fd, auto t){
    auto loop = co_await this_coro::executor;
    tcp::socket s(loop, tcp::v4(), fd);
    co_await s.async_wait(t, use_awaitable);
    s.release();
}// asyncio_wait_fd_rw()


