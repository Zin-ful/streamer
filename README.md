# Streamer
    Streamer allows you to host your own online web-streaming service for movies, tv, and other video media

# Media

    Movies and TV Shows are stored in media/tv and media/movies. They function slightly differently as in media/tv the server expects these series to be inside of their own directory.

    For example, if you have the old Tom and Jerry files, they will need to be in a subdirectory called 'Tom and Jerry' (or whatever name).
    This doesnt mean the name of the video files and its parent directory matter to each other, but they just have to be inside of their own folder.

    In media/movies, this isnt a requirement. In this directory you can just copy movies into it.

    NOTE: There are already directories for specific genres. These need to stay and are omitted by search. These directories are for genre sorting, but if no sorting is applied they are all searched.
    To prevent file naming issues each are assigned a code to make sure your free to pick whatever name without it being omitted.

# WARNINGS:

    This is not intended for pirated content.
    
    Supported formats are .mp4, and .mkv

    You are free to edit the index.html page however do not change the order of the <form></form> section. The order of http arguments are: 1. search 2. choice 3. sort
    The server expects their related ID and order to match whats specified in the code. So with that being said, change anything except for the content within the form tags for stability.
