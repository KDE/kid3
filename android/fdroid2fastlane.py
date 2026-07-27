#!/usr/bin/env python3
"""
Generate fastlane metadata from F-Droid YAML file.

Usage:
    python3 fdroid2fastlane.py --fdroidyaml kid3/packaging/f-droid/net.sourceforge.kid3.yml \
        --summary 'Edit audio file metadata' \
        --icon kid3/src/app/128-apps-kid3.png \
        --screenshot https://kid3.kde.org/images/ss_android_app.png \
        --fastlanezip fastlane-org.kde.kid3.zip
"""

import argparse
import re
import shutil
import urllib.request
import zipfile
from pathlib import Path
from tempfile import TemporaryDirectory

import yaml


def parse_description(description_text: str | list[str] | None) -> str:
    """
    Parse the description from F-Droid YAML.
    Handles both plain text and the |- formatted multi-line strings.
    """
    if description_text is None:
        return ""
    if isinstance(description_text, str):
        return description_text.strip()
    if isinstance(description_text, list):
        return "\n".join(description_text).strip()
    return str(description_text).strip()


def extract_metadata(fdroid_yaml_path: str) -> dict[str, str]:
    """
    Extract relevant metadata from F-Droid YAML file.
    Returns a dictionary with title, short_description, full_description.
    """
    with open(fdroid_yaml_path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)
    description = parse_description(data.get("Description", ""))
    short = ""
    if description:
        lines = [line.strip() for line in description.split("\n") if line.strip()]
        if lines:
            short = lines[0]
    else:
        description = data.get("Summary", "")
        short = description
    if len(short) > 80:
        short = short[:77] + "..."
    summary_yml = ""
    with open(fdroid_yaml_path, "r", encoding="utf-8") as f:
        for line in f:
            if not line.strip():
                break
            summary_yml += line
    return {
        "title": data.get("AutoName", ""),
        "short_description": short,
        "full_description": description,
        "summary_yml": summary_yml,
    }


def create_fastlane_structure(
    metadata: dict[str, str], app_id: str, summary="", icon_path="", screenshot_path=""
) -> TemporaryDirectory[str]:
    """
    Create the fastlane directory structure in a temporary directory.
    Returns the path to the temp directory.
    """
    temp_dir = TemporaryDirectory()
    root_path = Path(temp_dir.name)
    if metadata["summary_yml"]:
        yml_file = root_path / f"{app_id}.yml"
        yml_file.write_text(metadata["summary_yml"], encoding="utf-8")
    metadata_dir = root_path / app_id / "en-US"
    metadata_dir.mkdir(parents=True, exist_ok=True)
    title_file = metadata_dir / "title.txt"
    title_file.write_text(metadata["title"], encoding="utf-8")
    short_desc_file = metadata_dir / "short_description.txt"
    short_desc_file.write_text(
        summary if summary else metadata["short_description"], encoding="utf-8"
    )
    full_desc_file = metadata_dir / "full_description.txt"
    full_desc_file.write_text(metadata["full_description"], encoding="utf-8")
    images_dir = metadata_dir / "images"
    images_dir.mkdir(parents=True, exist_ok=True)
    if icon_path and Path(icon_path).exists():
        shutil.copy2(icon_path, images_dir / "icon.png")
    if screenshot_path:
        screenshots_dir = images_dir / "phoneScreenshots"
        screenshots_dir.mkdir(parents=True, exist_ok=True)
        if screenshot_path.startswith("http"):
            try:
                urllib.request.urlretrieve(
                    screenshot_path,
                    screenshots_dir / f"1-{metadata['title'].lower()}.png",
                )
            except urllib.request.HTTPError:
                print(f"Could not download {screenshot_path}")
        if Path(screenshot_path).exists():
            shutil.copy2(screenshot_path, screenshots_dir)
    return temp_dir


def create_fastlane_zip(
    temp_dir: TemporaryDirectory[str], output_zip_path: str
) -> None:
    """
    Create a zip file from the temporary directory structure.
    """
    with zipfile.ZipFile(output_zip_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        root_path = Path(temp_dir.name)
        for file_path in root_path.rglob("*"):
            if file_path.is_file():
                rel_path = file_path.relative_to(root_path)
                zipf.write(file_path, str(rel_path))


def main():
    parser = argparse.ArgumentParser(
        description="Generate fastlane metadata from F-Droid YAML file"
    )
    parser.add_argument("--fdroidyaml", required=True, help="Path to F-Droid YAML file")
    parser.add_argument("--summary", help="Short description", default="")
    parser.add_argument("--icon", help="Paths to icon file", default="")
    parser.add_argument("--screenshot", help="Path to screenshot file", default="")
    parser.add_argument(
        "--fastlanezip",
        required=True,
        help="Path where the output fastlane zip file should be created",
    )
    args = parser.parse_args()
    metadata = extract_metadata(args.fdroidyaml)
    m = re.match(r"^.*fastlane-([^/]+).zip$", args.fastlanezip)
    app_id = m.group(1) if m else "metadata/android"
    temp_dir = create_fastlane_structure(
        metadata,
        app_id,
        summary=args.summary,
        icon_path=args.icon,
        screenshot_path=args.screenshot,
    )
    create_fastlane_zip(temp_dir, args.fastlanezip)
    temp_dir.cleanup()
    print(f"Created fastlane zip archive: {args.fastlanezip}")


if __name__ == "__main__":
    main()
