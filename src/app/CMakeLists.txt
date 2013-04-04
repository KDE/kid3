include_directories(
  ../core/config
  ../core/utils
  ../core/tags
  ../core/model
  ../core/import
  ../gui/forms
  ../gui/dialogs
)

if (WITH_KDE)
  add_subdirectory(kde)
else (WITH_KDE)
  add_subdirectory(qt)
endif (WITH_KDE)
