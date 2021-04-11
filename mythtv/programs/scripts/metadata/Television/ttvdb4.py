#!/usr/bin/env python
# -*- coding: UTF-8 -*-

__title__ ="TheTVDB.com OpenAPI";
__author__="David Hampton"
__version__="1.0.0"

#base_url = "https://api4.thetvdb.com/v4"
base_url = "http://api4.thetvdb.com/v4"

import os
import sys

# Need to add to system path so that the generated openapi_client
# module can find its sub-modules.
for dir in sys.path:
    dir = os.path.join(dir, "MythTV")
    if os.path.isdir(dir):
        dir = os.path.join(dir, "ttvdb4")
        sys.path.append(dir)
        print("Added '%s' to search path." % dir)

# Verify that tvdb_api.py are available
try:
    from MythTV.ttvdb4 import openapi_client
except Exception as e:
    print("The ttvdb4 module should have been installed along with the\n"
          "MythTV python bindings.\n"
          "Error:(%s)" % e)
    sys.exit(1)

# Login
from openapi_client.api import login_api
from openapi_client.model.inline_object import InlineObject
from openapi_client.model.inline_response20026 import InlineResponse20026
from pprint import pprint

# Read user pin token here

configuration = openapi_client.Configuration(
    host = base_url,
)

# Enter a context with an instance of the API client
with openapi_client.ApiClient(configuration) as api_client:
    api_instance = login_api.LoginApi(api_client)
    inline_object = InlineObject(apikey="708401c2-b73e-4ce0-97ec-8ef3a5038c3a")
    try:
        api_response = api_instance.login_post(inline_object)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling LoginApi->login_post: %s\n" % e)

session_token = api_response.data.token

from openapi_client.api import series_api
from openapi_client.model.inline_response20039 import InlineResponse20039
from openapi_client.model.inline_response20040 import InlineResponse20040
import json # debug

configuration = openapi_client.Configuration(
    host = base_url,
    access_token = session_token
)

with openapi_client.ApiClient(configuration) as api_client:
    api_instance = series_api.SeriesApi(api_client)
    id = 76568
    try:
        api_response = api_instance.get_series_base(id)
        pprint(api_response)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_base: %s\n" % e)

    try:
        api_response = api_instance.get_series_extended(id, _preload_content=False)
        pprint(api_response.data)
        pprint(json.loads(api_response.data.decode('utf-8')))
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_extended: %s\n" % e)
#    try:
#        api_response = api_instance.get_series_extended(id)
#        pprint(api_response)
#    except openapi_client.ApiException as e:
#        print("Exception when calling SeriesApi->get_series_extended: %s\n" % e)
