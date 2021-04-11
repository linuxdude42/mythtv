# openapi_client.ArtworkApi

All URIs are relative to *https://api4.thetvdb.com/v4*

Method | HTTP request | Description
------------- | ------------- | -------------
[**get_artwork_base**](ArtworkApi.md#get_artwork_base) | **GET** /artwork/{id} | 
[**get_artwork_extended**](ArtworkApi.md#get_artwork_extended) | **GET** /artwork/{id}/extended | 
[**get_artwork_translation**](ArtworkApi.md#get_artwork_translation) | **GET** /artwork/{id}/translations/{language} | 


# **get_artwork_base**
> InlineResponse2002 get_artwork_base(id)



Returns artwork base record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import artwork_api
from openapi_client.model.inline_response2002 import InlineResponse2002
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
    api_instance = artwork_api.ArtworkApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_artwork_base(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling ArtworkApi->get_artwork_base: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse2002**](InlineResponse2002.md)

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

# **get_artwork_extended**
> InlineResponse2003 get_artwork_extended(id)



Returns artwork extended record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import artwork_api
from openapi_client.model.inline_response2003 import InlineResponse2003
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
    api_instance = artwork_api.ArtworkApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_artwork_extended(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling ArtworkApi->get_artwork_extended: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse2003**](InlineResponse2003.md)

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

# **get_artwork_translation**
> InlineResponse2004 get_artwork_translation(id, language)



Returns artwork translation record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import artwork_api
from openapi_client.model.inline_response2004 import InlineResponse2004
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
    api_instance = artwork_api.ArtworkApi(api_client)
    id = 1 # int | id
    language = "language_example" # str | language

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_artwork_translation(id, language)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling ArtworkApi->get_artwork_translation: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |
 **language** | **str**| language |

### Return type

[**InlineResponse2004**](InlineResponse2004.md)

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

