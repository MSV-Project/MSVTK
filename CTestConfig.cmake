set(CTEST_PROJECT_NAME "MSVTK")
set(CTEST_NIGHTLY_START_TIME "3:00:00 UTC")

set(CTEST_DROP_METHOD "http")
# See this issue for more details: http://public.kitware.com/Bug/view.php?id=12555
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=MSVTK")
set(CTEST_DROP_SITE_CDASH TRUE)
