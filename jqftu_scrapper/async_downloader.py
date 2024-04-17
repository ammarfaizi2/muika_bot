from tqdm.auto import tqdm
from io import BytesIO
from urllib.parse import urlparse
import asyncio
import httpx
import shutil
import os
import hashlib

async def maybe_coro(coro, *args, **kwargs):
    loop = asyncio.get_running_loop()

    if asyncio.iscoroutinefunction(coro):
        return await coro(*args, **kwargs)
    else:
        return await loop.run_in_executor(None, coro, *args, **kwargs)
	
class MultiConnectionDownloader:
    MINIMUM_PART_SIZE = 1024**2
    def __init__(
        self,
        session,
        *args,
        loop=None,
        progress_bar=None,
        **kwargs,
    ):
        self.session = session

        self.args = args
        self.kwargs = kwargs

        self.loop = loop or asyncio.new_event_loop()
        self.io_lock = asyncio.Lock()

        self.progress_bar = progress_bar

    async def download_part(
        self,
        io,
        start: int,
        end: int,
        progress_bar=None,
        future=None,
        pause_event=None,
    ):

        headers = self.kwargs.pop("headers", {})
        content_length = end
        position = start or 0

        is_incomplete = lambda: content_length is None or position < content_length
        is_downloading = lambda: (pause_event is None or not pause_event.is_set())

        while is_downloading() and is_incomplete():

            if content_length is None:
                if start is not None:
                    headers["Range"] = f"bytes={position}-"
            else:
                headers["Range"] = f"bytes={position}-{content_length}"

            try:
                async with self.session.stream(
                    *self.args, **self.kwargs, headers=headers
                ) as response:

                    content_length = (
                        int(response.headers.get("Content-Length", 0)) or None
                    )

                    if progress_bar is not None:

                        if content_length > 0:
                            progress_bar.total = content_length

                    async for chunk in response.aiter_bytes(8192):

                        chunk_size = len(chunk)

                        if self.progress_bar is not None:
                            self.progress_bar.update(chunk_size)

                        if progress_bar is not None:
                            progress_bar.update(chunk_size)

                        await self.write_to_file(
                            self.io_lock,
                            io,
                            position,
                            chunk,
                        )
                        position += chunk_size

                        if not is_downloading():
                            break

                    if content_length is None:
                        content_length = position

            except httpx.HTTPError:
                locks = ()

                if progress_bar is not None:
                    locks += (progress_bar.get_lock(),)
                if self.progress_bar is not None:
                    locks += (self.progress_bar.get_lock(),)

        if future is not None:
            future.set_result((start, position))

        return (start, position)

    @staticmethod
    async def write_to_file(
        lock: asyncio.Lock,
        io,
        position: int,
        data: bytes,
    ):

        async with lock:
            await maybe_coro(io.seek, position)
            await maybe_coro(io.write, data)
            await maybe_coro(io.flush)

    async def allocate_downloads(
        self,
        io,
        content_length: int = None,
        connections: int = 8,
        allocate_content_on_disk=False,
        pause_event=None,
    ):
        def iter_allocations():
            if content_length is None or content_length < self.MINIMUM_PART_SIZE:
                yield None, None
            else:
                chunk_size = content_length // connections
                for i in range(connections - 1):
                    yield i * chunk_size, (i + 1) * chunk_size - 1

                yield (connections - 1) * chunk_size, None

        if allocate_content_on_disk:
            async with self.io_lock:
                await maybe_coro(io.truncate, content_length)

        return await asyncio.gather(
            *(
                self.download_part(io, start, end, pause_event=pause_event)
                for start, end in iter_allocations()
            )
        )

    @staticmethod
    async def is_resumable(
        session,
        method,
        *args,
        **kwargs,
    ):
        headers = kwargs.pop("headers", {})

        headers["Range"] = "bytes=0-0"

        async with session.stream(method, *args, **kwargs) as response:
            return {
                "status_code": response.status_code,
                "headers": response.headers,
                "url": response.url,
            }
        
class AsyncDownloader:
    def __init__(self, url, connections):
        self.url = url
        self.connections = connections

    async def download_file(self, name):
        async with httpx.AsyncClient(timeout=60, follow_redirects=True, headers={"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.159 Safari/537.36"}) as session:
            progress_bar = tqdm(unit="B", unit_scale=True, unit_divisor=1024)
            head_response = await session.head(self.url)

            _dir, _name = name.rsplit('/', maxsplit=1)
            # Extract the filename from the URL's path and set the progress bar's total length
            content_length = int(head_response.headers.get("Content-Length", 0))
            progress_bar.total = content_length
            progress_bar.set_description(f"<= {_name!r}")

            # Open output file and initiate multi-connection download
            # full_path = os.path.join(directory, self.qualified_filename)
            if os.path.exists(name):
                os.remove(name)

            with open(name, "wb") as io:
                downloader = MultiConnectionDownloader(session, "GET", self.url, progress_bar=progress_bar)
                await downloader.allocate_downloads(io, content_length, connections=self.connections)

            md5_hash = hashlib.md5(name.encode()).hexdigest() + '.jpg'
            final = f"{_dir}/{md5_hash}"
            if os.path.exists(final):
                os.remove(final)
            shutil.move(name, final)

            # Close the progress bar after the download completes
            progress_bar.close()
            return md5_hash
        
class AsyncDownloaders:
    def __init__(self, urls, connections, session):
        self.urls = urls
        self.connections = connections
        self.session = session
    
    async def download_file(self, added, photo_dir):
        for url in self.urls:
            name = urlparse(url).path.rsplit('/', 1)[-1]
            progress_bar = tqdm(
                unit="B",
                unit_scale=True,
                unit_divisor=1024,
            )

            response_dict = await MultiConnectionDownloader.is_resumable(
                self.session, "GET", url
            )

            content_length = (
                int(response_dict["headers"].get("Content-Length", 0)) or None
            )
            io_object = BytesIO()

            progress_bar.total = content_length
            progress_bar.set_description(f"<= {name.rsplit('/', maxsplit=1)[-1]!r}")

            downloader = MultiConnectionDownloader(
                self.session,
                "GET",
                url,
                progress_bar=progress_bar,
            )

            await downloader.allocate_downloads(
                io_object,
                content_length if response_dict["status_code"] == 206 else None,
                connections=self.connections,
            )

            progress_bar.close()
            io_object.seek(0)

            md5_hash = hashlib.md5(io_object.getvalue()).hexdigest() + '.jpg'
            final = f"{photo_dir}/{md5_hash}"
            if os.path.exists(final):
                os.remove(final)
            
            with open(final, 'wb') as file:
                file.write(io_object.read())
            added.photos.append(f"{added.n}/{md5_hash}")