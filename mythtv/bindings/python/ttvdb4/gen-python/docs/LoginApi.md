# openapi_client.LoginApi

All URIs are relative to *https://api4.thetvdb.com/v4*

Method | HTTP request | Description
------------- | ------------- | -------------
[**login_post**](LoginApi.md#login_post) | **POST** /login | create an auth token


# **login_post**
> InlineResponse20026 login_post(inline_object)

create an auth token

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import login_api
from openapi_client.model.inline_object import InlineObject
from openapi_client.model.inline_response20026 import InlineResponse20026
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
    api_instance = login_api.LoginApi(api_client)
    inline_object = InlineObject(
        apikey="apikey_example",
        pin="pin_example",
    ) # InlineObject | 

    # example passing only required values which don't have defaults set
    try:
        # create an auth token
        api_response = api_instance.login_post(inline_object)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling LoginApi->login_post: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **inline_object** | [**InlineObject**](InlineObject.md)|  |

### Return type

[**InlineResponse20026**](InlineResponse20026.md)

### Authorization

[bearerAuth](../README.md#bearerAuth)

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details
| Status code | Description | Response headers |
|-------------|-------------|------------------|
**200** | response |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

