#include <stdio.h>
#include <stdlib.h>

int main() {
    // "test" 실행 파일 실행 후 결과를 읽어옴
    FILE *fp;
    char buffer[1024];
    
    char *busStop1 = "상인우방아파트";
    char *busStop2 = "경북대학교북문앞";
    char *programCode = "./test1";
    
    char temp[1024];
    
    snprintf(temp, sizeof(char)*1024, "%s %s %s", programCode, busStop1, busStop2);
    
    fp = popen(temp, "r"); // 외부 프로그램 실행
    if (fp == NULL) {
        printf("Failed to run command\n");
        return 1;
    }

    // 외부 프로그램에서 출력한 결과를 읽음
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer); // 결과를 출력하거나 처리함
    }

    pclose(fp); // 파일 포인터 닫기

    return 0;
}
