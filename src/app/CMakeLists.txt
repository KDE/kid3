include_directories(
  ../core/config
  ../core/utils
  ../core/tags
  ../core/model
  ../gui/forms
)

if (WITH_KDE)
  add_subdirectory(kde)
else (WITH_KDE)
  add_subdirectory(qt)
endif (WITH_KDE)
