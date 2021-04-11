# openapi_client.MoviesApi

All URIs are relative to *https://api4.thetvdb.com/v4*

Method | HTTP request | Description
------------- | ------------- | -------------
[**get_all_movie**](MoviesApi.md#get_all_movie) | **GET** /movies | 
[**get_movie_base**](MoviesApi.md#get_movie_base) | **GET** /movies/{id} | 
[**get_movie_extended**](MoviesApi.md#get_movie_extended) | **GET** /movies/{id}/extended | 
[**get_movie_translation**](MoviesApi.md#get_movie_translation) | **GET** /movies/{id}/translations/{language} | 


# **get_all_movie**
> InlineResponse20028 get_all_movie()



returns list of movie base records

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import movies_api
from openapi_client.model.inline_response20028 import InlineResponse20028
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
    api_instance = movies_api.MoviesApi(api_client)
    page = 3.14 # float | page number (optional)

    # example passing only required values which don't have defaults set
    # and optional values
    try:
        api_response = api_instance.get_all_movie(page=page)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling MoviesApi->get_all_movie: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **page** | **float**| page number | [optional]

### Return type

[**InlineResponse20028**](InlineResponse20028.md)

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

# **get_movie_base**
> InlineResponse20029 get_movie_base(id)



Returns movie base record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import movies_api
from openapi_client.model.inline_response20029 import InlineResponse20029
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
    api_instance = movies_api.MoviesApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_movie_base(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling MoviesApi->get_movie_base: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse20029**](InlineResponse20029.md)

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

# **get_movie_extended**
> InlineResponse20030 get_movie_extended(id)



Returns movie extended record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import movies_api
from openapi_client.model.inline_response20030 import InlineResponse20030
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
    api_instance = movies_api.MoviesApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_movie_extended(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling MoviesApi->get_movie_extended: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse20030**](InlineResponse20030.md)

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

# **get_movie_translation**
> InlineResponse2004 get_movie_translation(id, language)



Returns movie translation record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import movies_api
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
    api_instance = movies_api.MoviesApi(api_client)
    id = 1 # int | id
    language = "language_example" # str | language

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_movie_translation(id, language)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling MoviesApi->get_movie_translation: %s\n" % e)
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

