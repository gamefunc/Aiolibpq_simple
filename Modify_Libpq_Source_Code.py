""" GNU General Public License v3.0

postgresql libpq source code modify
    for Aiolibpq simple version

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
"""

import os, sys, re, shutil



PG_SOURCE_CODE_ROOT_PATH = r"C:\Downloads\postgres-REL_15_2"


LIBPQ_DIR_PATH = os.path.join(
    PG_SOURCE_CODE_ROOT_PATH,
    "src", "interfaces", "libpq"
)


class Modify_Libpq_Source_Code:
    def __init__(self) -> None:
        print("gamefunc: start modify")
        self.__modify_fe_connect_c()
        self.__modify_exports_txt()
        self.__modify_libpq_fe_h()
        print(f"gamefunc: modify end; "
            "report: github: gamefunc; "
            "qq: 32686647;")


    def __modify_libpq_fe_h(self) -> None:
        code_path = os.path.join(
            LIBPQ_DIR_PATH, "libpq-fe.h")
        self.__judge_code_file_exists(code_path)

        codes = """
// gamefunc add function start: bug report: github: gamefunc; qq: 32686647;
extern PGconn *PQgetSetupConn(const char *conninfo);
extern const char* PQgetUnresolvHost(PGconn* conn);
extern bool PQSetUnresolvHost(
		PGconn* conn, 
		const char* host_name, 
		const char* host_name_ip, int host_name_ip_len);
extern int PQconnectDBStart(PGconn *conn);
// gamefunc add function end: bug report: github: gamefunc; qq: 32686647;
        """
        out_text = ""
        with open(code_path, "r", 
                encoding="utf-8", newline="\n") as f:
            for line in f:
                out_text += line
                if "PQconnectStart(const char" in line:
                    out_text += f"\n{codes}\n"

        with open(code_path, "w", 
                encoding="utf-8", newline="\n") as f:
            f.write(out_text)



    def __modify_exports_txt(self) -> None:
        code_path = os.path.join(
            LIBPQ_DIR_PATH, "exports.txt")
        self.__judge_code_file_exists(code_path)

        out_text = ""
        max_num = 0
        with open(code_path, "r", 
                encoding="utf-8", newline="\n") as f:
            for line in f:
                unit = line.split()
                if len(unit) != 2: continue
                try:
                    if int(unit[1]) > max_num:
                        max_num = int(unit[1])
                except:
                    pass
                out_text += f"{line}"
        out_text += "\n"
        func_names = [
            "PQgetSetupConn",
            "PQgetUnresolvHost",
            "PQSetUnresolvHost",
            "PQconnectDBStart"
        ]
        for func_name in func_names:
            max_num += 1
            out_text += f"{func_name} {max_num}\n"

        with open(code_path, "w", 
                encoding="utf-8", newline="\n") as f:
            f.write(out_text)

        

    def __modify_fe_connect_c(self) -> None:
        code_path = os.path.join(
            LIBPQ_DIR_PATH, "fe-connect.c")
        self.__judge_code_file_exists(code_path)
        
        codes = """
// gamefunc add function start: bug report: github: gamefunc; qq: 32686647;
PGconn* PQgetSetupConn(const char *conninfo){
	PGconn *conn;
	conn = makeEmptyPGconn();
	if (conn == NULL) { return NULL; }
	if (!connectOptions1(conn, conninfo)){ return conn; }
	if (!connectOptions2(conn)){ return conn; }
	return conn;
}// PQgetSetupConn()


const char* PQgetUnresolvHost(PGconn* conn){
	for(int i = 0; i < conn->nconnhost; i++){
		pg_conn_host *ch = &conn->connhost[i];
		if((ch->type == CHT_HOST_NAME) 
				&& (strcmp(ch->host, DefaultHost) != 0)
				&& (ch->hostaddr == NULL)){
			return ch->host;
		}// if (not have host_addr) && (host != localhost)
	}// for i in range(nconnhost):
	return NULL;
}// PQgetUnresolvHost()


// host_name_ip_len: len("123.123") == 7;
bool PQSetUnresolvHost(
		PGconn* conn, 
		const char* host_name, 
		const char* host_name_ip, int host_name_ip_len){
	bool ok = false;
	for(int i = 0; i < conn->nconnhost; i++){
		if( (conn->connhost[i].type == CHT_HOST_NAME)
				&& (strcmp(conn->connhost[i].host, DefaultHost) != 0)
				&& (conn->connhost[i].hostaddr == NULL)){
			if(strcmp(conn->connhost[i].host, host_name) != 0){
				continue;
			}
			char *p = (char*)malloc(
				sizeof(char) * (host_name_ip_len + 1));
			if(!p){ return false; }
			memcpy(&p[0], &host_name_ip[0], host_name_ip_len);
			p[host_name_ip_len] = '\\0';
			conn->connhost[i].hostaddr = p;
			conn->connhost[i].type = CHT_HOST_ADDRESS;
			ok = true;
		}// if is need set host_name:
	}// for i in range(nconnhost):

	return ok;
}// PQSetUnresolvHost()

int PQconnectDBStart(PGconn *conn){
	return connectDBStart(conn);
}// PQconnectDBStart()
// gamefunc add function end: bug report: github: gamefunc; qq: 32686647;
        """
        with open(code_path, "a", 
            encoding="utf-8", newline="\n") as f:
            f.write(f"\n\n{codes}\n\n")



    def __judge_code_file_exists(self, code_path) -> None:
        if not os.path.exists(code_path):
            print(f"{code_path} not exists")
            raise
        if not os.path.exists(f"{code_path}.bak"):
            print(f"cp {code_path} -> {code_path}.bak")
            shutil.copyfile(code_path, f"{code_path}.bak")
        print(f"process: {code_path}")


Modify_Libpq_Source_Code()
