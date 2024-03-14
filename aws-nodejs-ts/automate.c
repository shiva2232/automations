#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000

char *concat(const char *str1, const char *str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char *result = malloc(len1 + len2 + 1); // +1 for the null terminator
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(result, str1);
    strcat(result, str2);

    return result;
}

int installDeps(){
    char pn[100];
    printf("Welcome to serverless setup!.\nEnter your project name: ");
    scanf("%s", pn);
    // installations
    printf(concat("serverless create -t aws-nodejs-typescript -n ", pn));
    system(concat("serverless create -t aws-nodejs-typescript -n ", pn));
    printf("\nInstalling serverless-offline dev dep. from npm\n");
    system("npm i --save-dev serverless-offline");
    printf("Installing prettifier dev dep. from npm\n");
    system("npm i --save-dev prettier");
    printf("Installing husky dev dep. from npm\n");
    system("npm i --save-dev husky");

    printf("adding prettier file to current directory...\n");
    const char *prettier_config = "{ \n"
                                  "  \"singleQuote\": true,\n"
                                  "  \"printWidth\": 120,\n"
                                  "  \"trailingComma\": \"es5\"\n"
                                  "}";
    FILE *config_file = fopen(".prettierrc", "w");
    if (config_file == NULL)
    {
        fprintf(stderr, "Error opening configuration file\n");
        return 1;
    }
    fputs(prettier_config, config_file);
    fclose(config_file);
    sleep(1);
    printf(".prettierrc file added\n");

    printf("creating packages directory in current path\n");
    system("mkdir packages");
    printf("done.\nThis program supports editing of package.json file with upto 1000 lines.\n");
    printf("Editing package.json file...\n");
    return 0;
}


    /***************************************************************
     * Addition of workspaces....
     **************************************************************/
int addWorkspace(){
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int in_workspaces_section = 0; // Flag to track if the "workspaces" section exists

    file = fopen("package.json", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Check if the "workspaces" section already exists
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"workspaces\": [") != NULL)
        {
            in_workspaces_section = 1; // Set flag if the section is found
            break;
        }
    }
    // Close the file
    fclose(file);

    // If "workspaces" section doesn't exist, proceed to insert it
    if (!in_workspaces_section)
    {

        char file_content[MAX_LINE_LENGTH] = ""; // Initialize with an empty string
        char *search_str = "\"scripts\": {";
        char *insert_str = "  \"workspaces\": [\n"
                           "    \"packages/*\"\n"
                           "  ],\n";
        int in_scripts_section = 0; // Flag to track if we're in the "scripts" section

        // Open the file for reading
        file = fopen("package.json", "r");
        if (file == NULL)
        {
            perror("Error opening file");
            return 1;
        }

        // Read the entire file content into memory
        while (fgets(line, sizeof(line), file))
        {
            strcat(file_content, line);
            // Check if the line contains the start of the "scripts" section
            if (!in_scripts_section && strstr(line, search_str) != NULL)
            {
                in_scripts_section = 1; // Set flag to indicate we're in the "scripts" section
            }
            // Check if we're in the "scripts" section and find its end
            if (in_scripts_section && strstr(line, "}") != NULL)
            {
                // Insert the "workspaces" line immediately after the closing curly brace of "scripts"
                strcat(file_content, insert_str);
                in_scripts_section = 0; // Reset flag
            }
        }

        // Close the file
        fclose(file);

        // Open the file for writing
        file = fopen("package.json", "w");
        if (file == NULL)
        {
            perror("Error opening file");
            return 1;
        }

        // Write the modified content back to the file
        fputs(file_content, file);

        // Close the file
        fclose(file);

        printf("The 'workspaces' section does not exist in package.json. Proceeding with insertion.\n");
    }else{
        printf("The 'workspaces' section already exists in package.json. No modifications needed.\n");
    }
    sleep(1);
    return 0;
}


int updateScripts(){
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char file_content[MAX_LINE_LENGTH] = ""; // Initialize with an empty string
    char *search_str = "\"scripts\": {";
    char *new_scripts = "    \"start:dev\": \"serverless offline --aws-profile alm-shopify-pos\",\n"
                        "    \"lint\": \"eslint --cache src --ext .ts\",\n"
                        "    \"lint:fix\": \"npm run lint -- --fix\",\n"
                        "    \"prettier-format\": \"prettier --config .prettierrc src//.ts --write\",\n"
                        "    \"prepare\": \"husky install\"\n";
    int in_scripts_section = 0; // Flag to track if we're in the "scripts" section

    // Open the file for reading
    file = fopen("package.json", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read the entire file content into memory
    while (fgets(line, sizeof(line), file)) {
        // Check if the line contains the start of the "scripts" section
        if (!in_scripts_section && strstr(line, search_str) != NULL) {
            in_scripts_section = 1; // Set flag to indicate we're in the "scripts" section
            strcat(file_content, line); // Append the line containing "{"
            // Skip the old script lines
            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, "}") != NULL) {
                    // Append the new script entries after the closing curly brace
                    strcat(file_content, new_scripts);
                    strcat(file_content, line); // Append the closing curly brace
                    in_scripts_section = 0; // Reset flag
                    break;
                }
            }
        } else {
            strcat(file_content, line); // Append other lines directly
        }
    }

    // Close the file
    fclose(file);
    sleep(1);

    // Open the file for writing
    file = fopen("package.json", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Write the modified content back to the file
    fputs(file_content, file);

    // Close the file
    fclose(file);

    printf("Done.\n");
    return 0;
}


int configEslint(){
    printf("Eslint configuration started...");
    system("npm init @eslint/config");
    printf("Eslint configured successfully...");
    return 0;
}

int main(int argc, char *argv[])
{
    installDeps();
    addWorkspace();
    updateScripts();
    configEslint();

    return 0;
}