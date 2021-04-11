# openapi_client.SeriesApi

All URIs are relative to *https://api4.thetvdb.com/v4*

Method | HTTP request | Description
------------- | ------------- | -------------
[**get_all_series**](SeriesApi.md#get_all_series) | **GET** /series | 
[**get_series_base**](SeriesApi.md#get_series_base) | **GET** /series/{id} | 
[**get_series_episodes**](SeriesApi.md#get_series_episodes) | **GET** /series/{id}/episodes/{season-type} | 
[**get_series_extended**](SeriesApi.md#get_series_extended) | **GET** /series/{id}/extended | 
[**get_series_translation**](SeriesApi.md#get_series_translation) | **GET** /series/{id}/translations/{language} | 


# **get_all_series**
> InlineResponse20038 get_all_series()



returns list of series base records

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import series_api
from openapi_client.model.inline_response20038 import InlineResponse20038
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
    api_instance = series_api.SeriesApi(api_client)
    page = 3.14 # float | page number (optional)

    # example passing only required values which don't have defaults set
    # and optional values
    try:
        api_response = api_instance.get_all_series(page=page)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_all_series: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **page** | **float**| page number | [optional]

### Return type

[**InlineResponse20038**](InlineResponse20038.md)

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

# **get_series_base**
> InlineResponse20039 get_series_base(id)



Returns series base record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import series_api
from openapi_client.model.inline_response20039 import InlineResponse20039
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
    api_instance = series_api.SeriesApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_series_base(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_base: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse20039**](InlineResponse20039.md)

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

# **get_series_episodes**
> InlineResponse20041 get_series_episodes(id, season_type)



Returns series episodes from the specified season type, default returns the episodes in the series default season type

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import series_api
from openapi_client.model.inline_response20041 import InlineResponse20041
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
    api_instance = series_api.SeriesApi(api_client)
    id = 1 # int | id
    season_type = "default" # str | season-type

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_series_episodes(id, season_type)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_episodes: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |
 **season_type** | **str**| season-type |
 **page** | **int**|  | defaults to 0

### Return type

[**InlineResponse20041**](InlineResponse20041.md)

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

# **get_series_extended**
> InlineResponse20040 get_series_extended(id)



Returns series extended record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import series_api
from openapi_client.model.inline_response20040 import InlineResponse20040
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
    api_instance = series_api.SeriesApi(api_client)
    id = 1 # int | id

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_series_extended(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_extended: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **int**| id |

### Return type

[**InlineResponse20040**](InlineResponse20040.md)

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

# **get_series_translation**
> InlineResponse2004 get_series_translation(id, language)



Returns series translation record

### Example

* Bearer (JWT) Authentication (bearerAuth):
```python
import time
import openapi_client
from openapi_client.api import series_api
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
    api_instance = series_api.SeriesApi(api_client)
    id = 1 # int | id
    language = "language_example" # str | language

    # example passing only required values which don't have defaults set
    try:
        api_response = api_instance.get_series_translation(id, language)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_translation: %s\n" % e)
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

