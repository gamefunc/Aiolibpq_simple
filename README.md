Aiolibpq_simple
=======

Aiolibpq simple example version for c++20 co_await with boost asio   

Author: gamefunc:
----------------
    website: https://www.gamefunc.top:9029
    github: https://github.com/gamefunc
    qq: 32686647
    weixin: gamefunc
    mail: fevefun@hotmail.com  
    
  
Example codes:
----------------
```c++
/* full in Aiolibpq_simple_test_unit.cpp */
awaitable<void> query_test_task(int i, std::string cmd){
    Aiolibpq aiopg_conn(
        "db_name", "gamefunc", "password", 
        "127.0.0.1", 5432);

    err = co_await aiopg_conn.connect();
    if(err){ throw 0; }

    err = co_await aiopg_conn.execute(cmd);
    if(err){ throw 0; }

    auto [order_key_list, rows_dict] = co_await aiopg_conn.fetchall();

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
```   
  
  
Test unit:
----------------
Aiolibpq_simple_test_unit.cpp  
    
    
How to use:
----------------
1. need modify postgresql source code to add get unresolv domain,   
  run "Modify_Libpq_Source_Code.py" (need set PG_SOURCE_CODE_ROOT_PATH);
2. then rebuild libpq c lib;  
3. include and link libpq;  
3. include and link boost asio;  
4. set(CMAKE_CXX_STANDARD_REQUIRED 20);  


Note:
----------------
1. if you don't want to modify pg source code and rebuild libpq;       
&#8195;you can parse conn_params string:     
&#8195;if ("host=" in conn_params) and ("hostaddr=" not in conn_params):       
&#8195;&#8195;get_domain_name_and_resolv_and_add_hostaddr()

2. see "Aiolibpq_simple.hpp"  coro::gen<PGresult*> fetchone();
```c++
  // want like: for(auto p: co_await fetchone()){ if(!p){break;} dodo(p); }
  // but co_yield coro task is not yet supported: so impl yourself;
  // see "Aiolibpq_simple.hpp"  coro::gen<PGresult*> fetchone();
  ...some code...
  co_await asyncio_wait_fd_rw(fd, tcp::socket::wait_read);
  ...some code...
  while(!PQisBusy(conn)){
    co_yield PQgetResult(conn);
  }// while(!PQisBusy(conn))
```  

3: default use std::format, need gcc13, clang14, msvc19.32,  
&#8195;&#8195;https://en.cppreference.com/w/cpp/23  
&#8195;&#8195;if not, use fmt::format() or delete that;


License:
----------------
GNU General Public License v3.0  
free to use and modify, but need keep the above information.  
  

          
