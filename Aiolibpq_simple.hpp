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

#ifndef __GAMEFUNC_AIOLIBPQ_FOR_CPP_SIMPLE__
#define __GAMEFUNC_AIOLIBPQ_FOR_CPP_SIMPLE__

// boost asio:
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

// cpp std:
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <tuple>
// https://en.cppreference.com/w/cpp/23 gcc13, clang14, msvc19.32:
#include <format>

// libpq:
#include <libpq-fe.h>


// boost asio:
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;
namespace ssl = boost::asio::ssl;




class Aiolibpq{
    /* Aiolibpq simple version by gamefunc qq 32686647:
    need modify postgresql source code,
        use Modify_Libpq_Source_Code.py,
        then build libpq c lib;
        
    if you don't want to modify pg source code,
    you can parse conn_params string,
        if ("host=" in conn_params) and ("hostaddr=" not in conn_params):
            get_domain_name_and_resolv_and_add_hostaddr()
    */
    public:
        PGconn *conn = nullptr;

        /* @brief Constructor 0:
        @param conn_params 
            Aiolibpq aio_pg("dbname=hanime "
                " user=gamefunc password=gamefunc "
                " hostaddr=127.0.0.1  port=5432 ");  
        https://www.gamefunc.top:9029/public/knowledge/postgresql_x_cn/libpq-connect.html */
        Aiolibpq(std::string conn_params);

        /* @brief Constructor 1: 
        @param  https://www.gamefunc.top:9029/public/knowledge/postgresql_x_cn/libpq-connect.html */
        Aiolibpq(std::string dbname = "hanime",
            std::string user = "gamefunc",
            std::string password = "gamefunc",
            std::string host = "www.gamefunc.top || 127.0.0.1",
            uint16_t port = 5432,
            std::string extra_params = "");

        ~Aiolibpq();

        Aiolibpq() = delete;

        Aiolibpq(const Aiolibpq &src) = delete;

        Aiolibpq(Aiolibpq &&src);

        void close_conn();


        /* @brief do async connect db;
        @return int: success ? 0 : !0; */
        awaitable<int> connect();


        /* @brief send sql cmd(auto commit):
        @param cmd: "select * from hanime_collection;"
        @return int success ? 0 : !0;  */
        awaitable<int> execute(std::string_view cmd);


        /* @brief get all query result; (after co_await execute(cmd));
        @return [
            vector<string>: order_key_list = [
                "col0_name", "col1_name", ...
            ],
            umap<string, vector<string>> rows_dict = {
                "col0_name": ["row0_col0_v", "row1_col0_v", ...],
                "col1_name": ["row0_col1_v", "row1_col1_v", ...],
                ...
            }
        ]; */
        awaitable<std::tuple<
            std::vector<std::string>,
            std::unordered_map<std::string, std::vector<std::string>> 
                > > fetchall();


        /* @brief setup fetchall() result dict;
        @param rows_dict 
        @param order_key_list
        @param rows 
        @return int  return int success ? 0 : !0; */
        int setup_result_rows_dict_key(
            std::unordered_map<std::string, std::vector<std::string>> &rows_dict,
            std::vector<std::string> &order_key_list,
            PGresult *rows);


        // /* @brief not yet support co_yield coro task:
        //    want like:
        //     for(auto p: co_await fetchone()){ if(!p){break;} dodo(p); }
        // @return PGresult* */
        // coro::gen<PGresult*> fetchone(){
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

    private:
        /* @brief await fd can write || read;
        @param fd: socket fd;
        @param t: tcp::socket:: wait_read || wait_write; */
        awaitable<void> asyncio_wait_fd_rw(
            auto fd, auto t);
};// class Aiolibpq{}





#endif// __GAMEFUNC_AIOLIBPQ_FOR_CPP_SIMPLE__