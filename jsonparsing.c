#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h" // cJSON 라이브러리 헤더

// 구조체 정의
typedef struct {
    char busNum[20];
    char current_stop[50];
    char remaining_stops[10];
    char arrival_time[10];
} BusInfo;

typedef struct{
    char name[50];
    char id[20];
    int forward;
    BusInfo buses[10]; // 최대 10개의 버스 정보를 저장할 수 있는 배열
    int numBuses; // 현재 버스 정보 개수
} StationInfo;

StationInfo stationInfo[5];
int cnt = 0;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
void Save_busInfo(char *jsonResponse, int n);
void Save_stationInfo(char *jsonResponse, int n);
void GetStationInfo(int n);
void GetStationId(int n);

int main(int argc, char* argv[]) {
    if (argc>1 && argc<6){
        for(int i=1; i<argc; i++) {
            snprintf(stationInfo[i-1].name, sizeof(char)*50, "%s", argv[i]);
        }
    }
    else {
        printf("Please put 5 or fewer parameters");
        exit(0);
    }
    
    int count = argc-1;
    
    for (int i=0; i<count; i++) {
        GetStationId(i);
        GetStationInfo(i);
    }

    return 0;
}

// Callback 함수: HTTP 응답 데이터를 받아서 처리하는 함수
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    char **response_ptr = (char **)userdata;
    
    *response_ptr = realloc(*response_ptr, realsize + 1);
    if (*response_ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }
    
    memcpy(*response_ptr, ptr, realsize);
    (*response_ptr)[realsize] = '\0';
    
    return realsize;
}

void Save_busInfo(char *jsonResponse, int n) {
    cJSON* root = NULL;
    root = cJSON_Parse(jsonResponse);
    if (root != NULL && cJSON_IsArray(root)) {
        int numBuses = cJSON_GetArraySize(root);
        stationInfo[n].numBuses = numBuses;
        printf("Number of buses: %d\n", numBuses);
        for (int i=0; i<numBuses; i++){
            cJSON *bus = cJSON_GetArrayItem(root, i);
            if (bus == NULL) {
                printf("Error: Couldn't get array item.\n");
                exit(0);
            }
            char* busNum = cJSON_GetObjectItem(bus, "name")->valuestring;
            cJSON *busArray = cJSON_GetObjectItem(bus, "bus");
            
            if (busArray == NULL) {
                printf("Error: Couldn't get array.\n");
                exit(0);
            }
            //          printf("busArray = : %s\n", cJSON_Print(busArray));
            
            cJSON *temp = cJSON_GetArrayItem(busArray, 0);
            
            BusInfo busInfo;
            const char* curStop = cJSON_GetObjectItem(temp, "현재정류소")->valuestring;
            const char* remainingStops = cJSON_GetObjectItem(temp, "남은정류소")->valuestring;
            const char* remainingTime = cJSON_GetObjectItem(temp, "도착예정시간")->valuestring;
            snprintf(busInfo.busNum, sizeof(char)*20, "%s", busNum);
            snprintf(busInfo.current_stop, sizeof(char)*50, "%s", curStop);
            snprintf(busInfo.remaining_stops, sizeof(char)*10, "%s", remainingStops);
            snprintf(busInfo.arrival_time, sizeof(char)*10, "%s", remainingTime);
            stationInfo[n].buses[i] = busInfo;
        }
    }
    else {
        printf("cJSON_Parse() failed\n");
        exit(0);
    }
    
    cJSON_Delete(root);
    return;
}

void Save_stationInfo(char *jsonResponse, int n) {
    cJSON* root = NULL;
    root = cJSON_Parse(jsonResponse);
    if (root != NULL && cJSON_IsArray(root)) {
        cJSON *station = cJSON_GetArrayItem(root, 0);
        if (station == NULL) {
            printf("Error: Couldn't get array item.\n");
            exit(0);
        }
        char* id = cJSON_GetObjectItem(station, "id")->valuestring;
        snprintf(stationInfo[n].id, sizeof(char)*20, "%s", id);
    }
    else {
        printf("cJSON_Parse() failed\n");
        exit(0);
    }
    
    cJSON_Delete(root);
    return;
}

void GetStationInfo(int n) {
    CURL *curl;
    CURLcode res;
    char *jsonResponse = NULL;
    char url[128];
    snprintf(url, sizeof(char)*128, "http://3.14.181.15:8080/station/%s", stationInfo[n].id);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url); // 요청할 URL 설정
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // 응답 데이터를 받을 callback 함수 설정
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsonResponse); // callback 함수에 전달할 사용자 데이터 설정
        
        res = curl_easy_perform(curl); // HTTP GET 요청 보내기
        
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } 
//      else {
//          printf("Received JSON response:\n%s\n", jsonResponse); // 받은 JSON 데이터 출력
//      }
        
        curl_easy_cleanup(curl); // libcurl 정리
    }
    
    Save_busInfo(jsonResponse,n);
    
    for(int j=0; j<stationInfo[n].numBuses; j++){
        printf("버스번호 : %s\n",stationInfo[n].buses[j].busNum);
        printf("현재정류소 : %s\n",stationInfo[n].buses[j].current_stop);
        printf("남은정류소 : %s\n",stationInfo[n].buses[j].remaining_stops);
        printf("도착예정시간 : %s\n",stationInfo[n].buses[j].arrival_time);
    }
    return;
}

void GetStationId(int n) {
    CURL *curl;
    CURLcode res;
    char *jsonResponse = NULL;
    char url[128];
    snprintf(url, sizeof(char)*128, "http://3.14.181.15:8080/station/search/%s", stationInfo[n].name);
    
    printf("%s\n", url);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url); // 요청할 URL 설정
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // 응답 데이터를 받을 callback 함수 설정
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsonResponse); // callback 함수에 전달할 사용자 데이터 설정
        
        res = curl_easy_perform(curl); // HTTP GET 요청 보내기
        
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } 
//      else {
//          printf("Received JSON response:\n%s\n", jsonResponse); // 받은 JSON 데이터 출력
//      }
        
        curl_easy_cleanup(curl); // libcurl 정리
    }
    
    Save_stationInfo(jsonResponse,n);
    
    printf("버스정류장 id : %s\n", stationInfo[n].id);
}
