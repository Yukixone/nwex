/**
 * @file file_receiver.c
 * @brief ファイルを受け取るプログラム
 */

#include "icslab2_net.h"

/**
 * @brief コマンドライン引数をパースする
 *
 * @param[in]  argc コマンドライン引数の数
 * @param[in]  argv コマンドライン引数の内容
 * @param[out] filename 受信した内容を保存するファイル名
 * @return
 * 正常にパースできたなら0、ヘルプを表示したなら1、引数が足りなかったなら2
 */
int ParseArgs(int argc, char **argv, char **filename) {
  if (argc == 1 || (argc == 2 && strncmp(argv[1], "-h", 2) == 0)) {
    printf("Usage: %s filename\n", argv[0]);
    return argc == 1 ? 2 : 1;
  }
  if (argc > 1)
    *filename = argv[1];
  return 0;
}

int main(int argc, char **argv) {
  int sock0; /* 待ち受け用ソケットディスクリプタ */
  int sock;  /* ソケットディスクリプタ */
  struct sockaddr_in serverAddr; /* サーバ＝自分用アドレス構造体 */
  struct sockaddr_in clientAddr; /* クライアント＝相手用アドレス構造体 */
  int addrLen;                   /* clientAddrのサイズ */

  char buf[BUF_LEN]; /* 受信バッファ */
  int n;             /* 受信バイト数 */
  int isEnd = 0;     /* 終了フラグ，0でなければ終了 */

  char *filename; /* 返送するファイルの名前 */
  int fd;         /* ファイルデスクリプタ */

  int yes = 1; /* setsockopt()用 */
  int result;

  /* コマンドライン引数の処理 */
  result = ParseArgs(argc, argv, &filename);
  if (result == 1) {
    return 0;
  } else if (result == 2) {
    return 1;
  }
  printf("filename: %s\n", filename);

  /* STEP 1: TCPソケットをオープンする */
  if ((sock0 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socke");
    return 1;
  }

  /* sock0のコネクションがTIME_WAIT状態でもbind()できるように設定 */
  setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

  /* STEP 2: クライアントからの要求を受け付けるIPアドレスとポートを設定する */
  memset(&serverAddr, 0, sizeof(serverAddr));   /* ゼロクリア */
  serverAddr.sin_family = AF_INET;              /* Internetプロトコル */
  serverAddr.sin_port = htons(TCP_SERVER_PORT); /* 待ち受けるポート */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */

  /* STEP 3: ソケットとアドレスをbindする */
  if (bind(sock0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind");
    return 1;
  }

  /* STEP 4: コネクションの最大同時受け入れ数を指定する */
  if (listen(sock0, 5) != 0) {
    perror("listen");
    return 1;
  }

  while (!isEnd) { /* 終了フラグが0の間は繰り返す */

    /* STEP 5: クライアントからの接続要求を受け付ける */
    printf("waiting connection...\n");
    addrLen = sizeof(clientAddr);
    sock = accept(sock0, (struct sockaddr *)&clientAddr, (socklen_t *)&addrLen);
    if (sock < 0) {
      perror("accept");
      return 1;
    }

    /* STEP 7: 出力先のファイルをオープン */
    fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
      perror("open");
      return 1;
    }

    /* 受信した内容をファイルに書き込む */
    while ((n = read(sock, buf, BUF_LEN)) > 0) {
      write(fd, buf, n);
    }

    /* STEP 9: 通信用のソケットのクローズ */
    close(sock);
    printf("closed\n");
  }

  /* STEP 10: 待ち受け用ソケットのクローズ */
  close(sock0);

  return 0;
}

/* vim: set ff=unix fenc=utf-8 : */
