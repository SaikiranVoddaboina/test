
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define API_KEY "your_openweather_api_key"
#define API_URL "http://api.openweathermap.org/data/2.5/weather?q="
#define MAX_FAVOURITES 3

typedef struct {
    char city[50];
} City;

City favourites[MAX_FAVOURITES];
int fav_count = 0;

struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + total_size + 1);
    if (ptr == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->memory[mem->size] = 0;

    return total_size;
}

void fetch_weather_details(const char *city) {
    CURL *curl;
    CURLcode res;
    char url[256];
    struct MemoryStruct chunk = {0};

    snprintf(url, sizeof(url), "%s%s&appid=%s", API_URL, city, API_KEY);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Weather details for %s:\n%s\n", city, chunk.memory);
        }

        curl_easy_cleanup(curl);
        free(chunk.memory);
    }
    curl_global_cleanup();
}

void add_city_to_favourites(const char *city) {
    if (fav_count >= MAX_FAVOURITES) {
        printf("Favourites list is full. Remove a city to add a new one.\n");
        return;
    }
    strcpy(favourites[fav_count++].city, city);
    printf("%s added to favourites.\n", city);
}

void list_favourite_cities() {
    if (fav_count == 0) {
        printf("No favourite cities found.\n");
        return;
    }
    for (int i = 0; i < fav_count; i++) {
        printf("%d. %s\n", i + 1, favourites[i].city);
        fetch_weather_details(favourites[i].city);
    }
}

void update_favourite_city(int index, const char *new_city) {
    if (index < 1 || index > fav_count) {
        printf("Invalid index. Choose a valid city number.\n");
        return;
    }
    strcpy(favourites[index - 1].city, new_city);
    printf("City updated to %s.\n", new_city);
}

void menu() {
    printf("\nMenu:\n");
    printf("1. Search Weather Details\n");
    printf("2. Add City to Favourites\n");
    printf("3. List Favourite Cities\n");
    printf("4. Update Favourite Cities\n");
    printf("5. Exit\n");
    printf("Choose an option: ");
}

int main() {
    int choice;
    char city[50];
    int index;

    while (1) {
        menu();
        scanf("%d", &choice);
        getchar(); // consume newline

        switch (choice) {
        case 1:
            printf("Enter city name: ");
            fgets(city, sizeof(city), stdin);
            city[strcspn(city, "\n")] = '\0';
            fetch_weather_details(city);
            break;

        case 2:
            printf("Enter city name to add to favourites: ");
            fgets(city, sizeof(city), stdin);
            city[strcspn(city, "\n")] = '\0';
            add_city_to_favourites(city);
            break;

        case 3:
            list_favourite_cities();
            break;

        case 4:
            printf("Enter the index of the city to update: ");
            scanf("%d", &index);
            getchar(); // consume newline
            printf("Enter the new city name: ");
            fgets(city, sizeof(city), stdin);
            city[strcspn(city, "\n")] = '\0';
            update_favourite_city(index, city);
            break;

        case 5:
            printf("Exiting application.\n");
            return 0;

        default:
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}

