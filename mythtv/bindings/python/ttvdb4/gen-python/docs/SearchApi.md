# openapi_client.SearchApi

All URIs are relative to *https://api4.thetvdb.com/v4*

Method | HTTP request | Description
------------- | ------------- | -------------
[**get_search_results**](SearchApi.md#get_search_results) | **GET** /search | 


# **get_search_results**
> InlineResponse20034 get_search_results()



Returns a search result record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import search_api
from openapi_client.model.inline_response20034 import InlineResponse20034
from pprint import pprint
# Defining the host is optional and defaults to https://api4.thetvdb.com/v4
# See configuration.py for a list of all supported configuration parameters.
configuration = openapi_client.Configuration(
    host = "https://api4.thetvdb.com/v4"
)

# The client must configure the authentication and authorization parameters
# in accordance with the API server security policy.
# Examples for each auth method are provided below, use the example that
# satisfies your auth use case.

# Configure Bearer authorization (JWT): bearerAuth
configuration = openapi_client.Configuration(
    access_token = 'YOUR_BEARER_TOKEN'
)

# Enter a context with an instance of the API client
with openapi_client.ApiClient(configuration) as api_client:
    # Create an instance of the API class
    api_instance = search_api.SearchApi(api_client)
    q = "q_example" # str | additional search query param (optional)
    query = "query_example" # str | additional search query param (optional)
    type = "type_example" # str | restrict results to entity type movie|series|person|company (optional)
    year = 3.14 # float | restrict results to a year for movie|series (optional)
    offset = 3.14 # float | offset results (optional)

    # example passing only required values which don't have defaults set
    # and optional values
    try:
        api_response = api_instance.get_search_results(q=q, query=query, type=type, year=year, offset=offset)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SearchApi->get_search_results: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **q** | **str**| additional search query param | [optional]
 **query** | **str**| additional search query param | [optional]
 **type** | **str**| restrict results to entity type movie|series|person|company | [optional]
 **year** | **float**| restrict results to a year for movie|series | [optional]
 **offset** | **float**| offset results | [optional]

### Return type

[**InlineResponse20034**](InlineResponse20034.md)

### Authorization

[bearerAuth](../README.md#bearerAuth)

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details
| Status code | Description | Response headers |
|-------------|-------------|------------------|
**200** | response |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

