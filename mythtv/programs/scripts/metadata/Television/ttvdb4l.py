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
        dir = os.path.join(dir, "ttvdb4l")
        sys.path.append(dir)
        print("Added '%s' to search path." % dir)

# Verify that tvdb_api.py are available
try:
    from MythTV.ttvdb4l import openapi_client
except Exception as e:
    print("The ttvdb4l module should have been installed along with the\n"
          "MythTV python bindings.\n"
          "Error:(%s)" % e)
    sys.exit(1)
from openapi_client.rest import ApiException
from pprint import pprint

# Read user pin token here

session_token = "None Yet"
configuration = openapi_client.Configuration(
    host = base_url
)

# Enter a context with an instance of the API client
with openapi_client.ApiClient(configuration) as api_client:
    api_instance = openapi_client.LoginApi(api_client)
    inline_object = openapi_client.InlineObject(apikey="708401c2-b73e-4ce0-97ec-8ef3a5038c3a")
    try:
        api_response = api_instance.login_post(inline_object)
        #pprint(api_response)
    except ApiException as e:
        print("Exception when calling LoginApi->login_post: %s\n" % e)

session_token = api_response.data.token

import json # debug

configuration = openapi_client.Configuration(
    host = base_url,
    access_token = session_token
)

with openapi_client.ApiClient(configuration) as api_client:
    series_id = 76568
    #series_id = 71470

    # Use series_extended call to get season information
    series_instance = openapi_client.SeriesApi(api_client)
    try:
        series_extended = series_instance.get_series_extended(series_id)
        #pprint(series_extended)
    except openapi_client.ApiException as e:
        print("Exception when calling SeriesApi->get_series_extended: %s\n" % e)
    print("Series %s, Id %d" % (series_extended.data.name, series_extended.data.id))

    # Grab each season (unordered)
    season_instance = openapi_client.SeasonsApi(api_client)
    for season in series_extended.data.seasons:
        if season.name != "Aired Order":
            continue
        print("  Season %d, name %s, id %d (series id %d)" %
              (season.number, season.name, season.id, season.series_id))

        try:
            season_response = season_instance.get_season_extended(season.id)
            #pprint(season_response)
        except ApiException as e:
            print("Exception when calling SeasonsApi->get_season_extended: %s\n" % e)

        # Grab each episode. They seem to be listed twice (with the
        # exact same data) in the result of the seasons call, so
        # filter out duplicates.
        episode_instance = openapi_client.EpisodesApi(api_client)
        episodes_processed = []
        for episode in season_response.data.episodes:
            if episode.season_number != season.number:
                continue
            if episode.number in episodes_processed:
                continue
            episodes_processed.append(episode.number)
            try:
                episode_response = episode_instance.get_episode_base(episode.id)
                #pprint(episode_response)
            except ApiException as e:
                print("Exception when calling EpisodesApi->get_episode_base: %s\n" % e)
            ep_info = episode_response.data
            print("    Episode %d, name %s, id %d" %
                  (ep_info.number, ep_info.name, ep_info.id))

            episode_trans_response = episode_instance.get_episode_translation(episode.id, 'eng')
            print("      Overview [eng]: %s" % episode_trans_response.data.overview)
            episode_trans_response = episode_instance.get_episode_translation(episode.id, 'deu')
            print("      Overview [deu]: %s" % episode_trans_response.data.overview)
            episode_trans_response = episode_instance.get_episode_translation(episode.id, 'fra')
            print("      Overview [fra]: %s" % episode_trans_response.data.overview)
