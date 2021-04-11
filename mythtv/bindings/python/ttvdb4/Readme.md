The TVDB API spec can be download from:

https://app.swaggerhub.com/apis/thetvdb/tvdb-api_v_4/

Download the latest version of the spec by clicking on the "Export"
button in the upper right of the page, and selecting "Download API"
then "YAML Unresolved".

Generate the code stubs with the command:

    openapi-generator-cli generate \
        -g python \
        -o gen-python \
        -i thetvdb-tvdb-api_v_4-4.3.0-swagger.yaml
    openapi-generator-cli generate \
        -g python-legacy \
        -o gen-python-legacy \
        -i thetvdb-tvdb-api_v_4-4.3.0-swagger.yaml
