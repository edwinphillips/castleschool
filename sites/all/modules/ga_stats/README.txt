# Google Analytics Statistics

Provides views integration for the Google Analytics Data Export API by caching
the data to the Drupal database.

This allows you to use Google Analytics directly through views to do things
like Most Popular, Most Read etc. You can combine the statistics to create
interesting results.

After you install the module - go to /admin/config/services/ga_stats to
configure your Google Analytics account information and begin pulling
statistics.

# Maintainers

@author jcaldwell aka jec006

# Configuration

1. Go to https://console.developers.google.com
2. Create a new project
  - in the API section enable Analytics API
  - in the 'credentials' section click on 'Create new Client ID' and choose 'Service account' with the key type P12
  - save the P12 key and Client ID
3. On your site go to /admin/config/media/file-system and make sure that the 'Private File System Path' is configured
4. Go to /admin/config/services/ga_stats/auth and upload your P12 key and set the email address (it's the same as Client ID which you copied earlier from console.developers.google.com)
5. Add Client ID email into your google analytics account with permissions Read/Analyze
