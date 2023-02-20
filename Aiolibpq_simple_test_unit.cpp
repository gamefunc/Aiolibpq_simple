/* GNU General Public License v3.0

Aiolibpq simple version test unit

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

#include "Aiolibpq_simple.hpp"
#include <chrono>
using boost::asio::steady_timer;

std::vector<std::string> QUERY_CMDS{
    "select * from hanime_collection;",
    "select cover_name from hanime_collection;"
};



awaitable<void> asyncio_sleep_ms(uint64_t ms) {
    auto loop = co_await this_coro::executor;
    steady_timer t(loop);
    t.expires_after(std::chrono::milliseconds(ms));
    co_await t.async_wait(use_awaitable);
}// asyncio_sleep_ms()




awaitable<void> query_test_task(int i, std::string cmd){
    auto loop = co_await this_coro::executor;
    bool ok = false;
    int err = 0;

    Aiolibpq aiopg_conn(
        "hanime", "gamefunc", "password", 
        "10.2.0.8", 5432);

    err = co_await aiopg_conn.connect();
    if(err){ 
        std::cout << std::format(
            "task_{}: connect() fail, "
            "err_num: {}; "
            "maybe need set your server config max_connections;\n", 
            i, err
        );
        throw 0; 
    }

    err = co_await aiopg_conn.execute(cmd);
    if(err){ 
        std::cout << std::format(
            "task_{}: execute({}) fail; "
            "err_num: {};\n", 
            i, cmd, err
        );
        throw 0; 
    }

    auto [order_key_list, rows_dict] = 
        co_await aiopg_conn.fetchall();

    std::cout << std::format(
        "task_{}: "
        "num_of_cols: {}; "
        "num_of_rows: {}; "
        "rows_dict[\"cover_name\"][{}]: {}\n",
        i,
        rows_dict.size(),
        rows_dict["cover_name"].size(),
        i,
        rows_dict["cover_name"][i]
    );

    // aiopg_conn.close_conn();
}// query_test()


awaitable<void> main_start(){
    auto loop = co_await this_coro::executor;
    for(int i = 1; i < 1000; i++){
        co_spawn(
            loop, 
            query_test_task(i, QUERY_CMDS[i % QUERY_CMDS.size()]), 
            detached);
        if(i % 120 == 0){ co_await asyncio_sleep_ms(5 * 1000); }
    }// for i in range(n)
}// main_start()


int main(){
    std::cout << "gamefunc: qq: 32686647: "
        "run Aiolibpq_simple_test_unit; \n";
    try{
        boost::asio::io_context ioc(1);
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ ioc.stop(); });
        co_spawn(ioc, main_start(), detached);
        ioc.run();
    }catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
    return 0;    
}// main()