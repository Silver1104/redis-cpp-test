import requests
from bs4 import BeautifulSoup
from yt_dlp import YoutubeDL
from urllib.parse import urljoin

def find_embed_url(page_url):
    headers = {
        "User-Agent": "Mozilla/5.0"
    }

    response = requests.get(page_url, headers=headers, timeout=30)
    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")

    iframe = soup.find("iframe")
    if iframe and iframe.get("src"):
        return urljoin(page_url, iframe["src"])

    return None

def download_video(video_url):
    ydl_opts = {
        "format": "best",
        "outtmpl": "%(title)s.%(ext)s",
        "noplaylist": True,
    }

    with YoutubeDL(ydl_opts) as ydl:
        ydl.download([video_url])

def main():
    page_url = input("Enter webpage URL containing embedded video: ").strip()

    try:
        embed_url = find_embed_url(page_url)

        if embed_url:
            print(f"Found embed URL:\n{embed_url}\n")
            print("Starting download...")
            download_video(embed_url)
            print("Download completed.")
        else:
            print("No iframe/embed video found on the page.")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()