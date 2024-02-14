from html2image import Html2Image
from bs4 import BeautifulSoup as bs
from bs4.element import Tag
import os, json
import pykakasi
from uuid import uuid4
import shutil
from pySmartDL import SmartDL
from functools import partial, wraps
from typing import Callable
import asyncio
import httpx
import re

client = httpx.AsyncClient(headers={
    'accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7',
    'accept-language': 'en-US,en;q=0.9,id;q=0.8,mt;q=0.7',
    'cache-control': 'max-age=0',
    'sec-ch-ua': '"Not A(Brand";v="99", "Microsoft Edge";v="121", "Chromium";v="121"',
    'sec-ch-ua-mobile': '?0',
    'sec-ch-ua-platform': '"Windows"',
    'sec-fetch-dest': 'document',
    'sec-fetch-mode': 'navigate',
    'sec-fetch-site': 'same-origin',
    'sec-fetch-user': '?1',
    'upgrade-insecure-requests': '1',
    'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36 Edg/121.0.0.0',
})

def aiowrap(func: Callable) -> Callable:
    @wraps(func)
    async def run(*args, loop=None, executor=None, **kwargs):
        if loop is None:
            loop = asyncio.get_event_loop()
        pfunc = partial(func, *args, **kwargs)
        return await loop.run_in_executor(executor, pfunc)

    return run

class StationModifier:
    def __init__(self, html_content, class_name='ib-station-name'):
        self.soup = bs(html_content, 'html.parser')
        self.class_name = class_name
    
    def duplicate_elements_and_modify(self):
        elements = self.soup.find_all('div', class_=self.class_name)
        
        filter_res = [
            elem for elem in elements 
            if elem.find('span', attrs={'style': re.compile('display:inline-block')})
        ]

        if not filter_res:
            return None
        
        if len(filter_res) < 2:
            return {'layout': filter_res[0], 'png_list': []}
            
        png_list = []
        tot = len(filter_res[0].find('span', attrs={'style': re.compile('display:inline-block')}).find_all('span', attrs={'style': re.compile('vertical-align:middle')}))
        target_div = filter_res[0].find('div', class_='fn org')
        for elem in filter_res[1:]:
            spans = elem.find_all('span', attrs={'style': re.compile('display:inline-block')})
            for item in spans:
                _fall = item.find_all('span')
                alpha, num = _fall[0].text, _fall[1].text 
                tot += 1  
                if tot % 8 == 0:
                    br_tag = self.soup.new_tag("br")
                    target_div.insert(1, br_tag)
                if f"{alpha}{num}.png" not in png_list:
                    target_div.insert(1, item)
                    png_list.append(f"{alpha}{num}.png")
        find_all_br = target_div.find_all('br')
        if len(find_all_br) > 1:
            last_br_tag = find_all_br[-2]  
            last_br_tag.extract()
        print('png list 2')
        print(png_list)
        return {'layout': filter_res[0], 'png_list': png_list, 'total_items': tot}
   
def char_is_hiragana(c) -> bool:
    return u'\u3040' <= c <= u'\u309F'

def string_is_hiragana(s: str) -> bool:
    return all(char_is_hiragana(c) for c in s)

hti = Html2Image(size=(1920, 1080), custom_flags=['--virtual-time-budget=10000', '--hide-scrollbars', '--no-sandbox'])
async def get(url):
    return (await client.get(url)).text

async def data_stasiun(stasiun):
    _wiki_url, _results, kks = 'https://en.wikipedia.org{url}', [], pykakasi.kakasi()
    for data in bs(await get(stasiun), 'html.parser').find('table', class_=re.compile('wikitable', re.IGNORECASE)).find('tbody').find_all('tr')[1:-1]:
        if len(data.find_all('td')) > 1:
            _results_json = {}
            _numbering = data.find('span', attrs={'style': re.compile('vertical-align:middle')})
            _T = _numbering.find_all('span')[0].text
            _N = _numbering.find_all('span')[1].text
            _num = f"{_T}{_N.zfill(2)}"
            _url_stas = data.find('a', attrs={'title': re.compile('Station')})
            _results_json['n'] = _num
            _results_json['romaji'] = _url_stas['title'].split(' Station')[0].lower()
            
            _kanji = (data.find('td', attrs={'style': re.compile('right')}) or data.find('td', attrs={'align': 'right'}) or data.find('td', attrs={'align': 'center'})).find_previous_sibling('td')
            if _kanji.find('sup'):
                _kanji.find('sup').extract()            
            _results_json['kanji'] = _kanji.get_text(strip=True)

            _kanji = kks.convert(_results_json['kanji'])[0]
            _results_json['hiragana'] = _kanji['hira']
            _results_json['katakana'] = _kanji['kana']
            _results_json['q_img'] = f"{_num}.png"
            _results_json['url_station'] = _wiki_url.format(url=_url_stas['href'])
            _plain_list = data.find('div', class_='plainlist')
            print(_results_json['romaji'])
            if _plain_list:
                _photo = []
                for _u in _plain_list.find_all('li'):
                     _a = _u.find_all('a')
                     if not _a or _a[0]['href'].endswith('.svg') or '/wiki/' not in _a[0]['href']:
                         continue
                     _p = bs(await get(_wiki_url.format(url=_a[-1]['href'])), 'html.parser')
                     _po = [thum.find('img')['src'] for thum in _p.find('table', class_='infobox').find('tbody').find_all('tr') if thum.find('div', class_='infobox-caption')]
                     if not _po:
                         continue
                     _photo.append(f"https:{change_image_size(_po[0], '1200')}")
                _results_json['photos'] = _photo
            else:
                 _p = bs(await get(_results_json['url_station']), 'html.parser')
                 thumb_v1 = _p.find('div', class_='thumb')
                 thumb_v2 = _p.find('span', class_='mw-default-size')
                 thum = _p.find_all('figure', class_='mw-default-size') or (thumb_v1.find_all('a') if thumb_v1 else []) or (thumb_v2.find_all('a') if thumb_v2 else [])
                 _results_json['photos'] = [f"https:{change_image_size(photo.find('img')['src'], '1200')}" for photo in thum if thum]
                 print(_results_json['photos'])
            _results.append(_results_json)
    return sorted(_results, key=lambda x: x['n'])

def zip_folder(directory_to_zip, output_zip_file):
    try:
        shutil.make_archive(output_zip_file.replace('.zip', ''), 'zip', directory_to_zip)
        print(f"Directory '{directory_to_zip}' has been successfully zipped into '{output_zip_file}'")
    except Exception as e:
        print(f"An error occurred while zipping the directory: {e}")

def change_image_size(url, new_size):
    return re.sub(r"/(\d+)px-", f"/{new_size}px-", url)    

@aiowrap
def downloader_smart(uri, folder):
    SmartDL(change_image_size(uri, '1200'), f"{folder}/{uuid4().hex}.jpg", progress_bar=False, request_args={'headers': {'User-Agent': 'CoolBot/0.0 (https://hmpbi.my.id/coolbot/; coolbot@gmail.com)'}}, verify=False).start(blocking=False)
    
async def stasiun_to_img(url_stas, all_stas=True, name=None, fsize=None, zoom=None, only_json=False, dir=None):
    utas_name = url_stas.rsplit('/', maxsplit=1)[-1]
    output_dir = f"{dir}/{utas_name}"

    if all_stas and not os.path.isdir(output_dir):
        os.makedirs(output_dir)
        os.makedirs(f"{output_dir}/photos")

    if not all_stas:
        output_dir = output_dir.rsplit('/', maxsplit=1)[0]
    _json_name = f"{output_dir}/{utas_name}.json"
    
    if all_stas and os.path.isfile(_json_name) and not only_json:
        with open(_json_name, 'r', encoding='utf-8') as stas_rea:
            stasiuns = json.load(stas_rea)
    elif (all_stas and not os.path.isfile(_json_name)) or only_json:
        stasiuns = await data_stasiun(url_stas)
        with open(_json_name, 'w', encoding='utf-8') as stas_save:
            json.dump(stasiuns, stas_save, indent=3, ensure_ascii=False)
        print("station's data saved to json!")
        if only_json:
            return _json_name
    else:
        stasiuns = [{'url_station': url_stas, 'q_img': f'{name}.png'}]

    for data in stasiuns:
        stasiun = StationModifier(await get(data['url_station'])).duplicate_elements_and_modify()
        hl = stasiun['layout']
        _fn_org = hl.find(class_='fn org')
        _br = _fn_org.find_all('br')
        _bg_black = _fn_org.find('span', attrs={'style': re.compile('background:black')})
        stas = hl.find('br').next_sibling
        two_lines = 0
        if _br:
            two_lines += len([x for x in _br if x.next_sibling.text.endswith('Station')])
            for n, br in enumerate(_br):                
                if ((two_lines < 2 and len(_br) > 1 and n == len(_br) - 1) or (two_lines > 1 and n == 0)):
                    stas = br.next_sibling
            if stas.name == 'big':
                stas = stas.text
        else:
            stas = hl.find('br').next_sibling
        
        jepun = hl.find(class_='nickname').get_text(strip=True)
        
        fsize = 60
        zoom_level = 470 if (two_lines  > 1 or _bg_black) else 650
        
        if len(jepun) >=5:
            zoom_level = 475
        if _bg_black and len(jepun) >= 8:
            zoom_level = 450
        
        if fsize:
            fsize = fsize
        if zoom:
            zoom_level = zoom

        if isinstance(stas, Tag):
            jap = f'''<div class="kanji" style="font-size: 80px">{stas.get_text(strip=True)}</div>'''
            stas = hl.find(class_='fn org').text
        else:
            jap = f'''<div class="kanji" style="font-size:{fsize}px">{jepun}</div>'''

        new = bs(str(hl).replace(stas, jap).replace('駅', '').replace('//upload.', 'https://upload.'), 'html.parser')
        new.find(class_='nickname').decompose()
        html = '<center>' + str(new) + '</center>'
        hti.output_path = output_dir
        hti.screenshot(html_str=html, css_str=f'body {{ zoom:{zoom_level}% }}', save_as=data['q_img']) # 650, 2 lines: 470
        if all_stas:
            photos_folder = f"{output_dir}/photos/{data['n']}"
            if not os.path.isdir(photos_folder):
                os.makedirs(photos_folder)
            for thumb in data['photos']:
                await downloader_smart(thumb, photos_folder)
    zipped = f"{output_dir}.zip"
    if all_stas:
        zip_folder(output_dir, zipped)
    return zipped

# zoom=440, fsize=40 https://en.wikipedia.org/wiki/Takanawa_Gateway_Station
tokyo = 'https://en.wikipedia.org/wiki/Tokyo_Metro_Tozai_Line'
yamato = 'https://en.wikipedia.org/wiki/Yamanote_Line'
keikyu = 'https://en.m.wikipedia.org/wiki/Keiky%C5%AB_Main_Line'
# stasiun_to_img('https://en.m.wikipedia.org/wiki/Keiky%C5%AB_Higashi-kanagawa_Station', all_stas=False, name='tokyo_8_v4', zoom=475) # zoom=475) # , fsize=40)
# stasiun_to_img(keikyu)
# print(data_stasiun('https://en.wikipedia.org/wiki/Yokosuka_Line', num='T', url_station=True))
# stasiun_to_img('https://en.wikipedia.org/wiki/Shibuya_Station', all_stas=False, name='tokyo_8_v5') # zoom=475) # , fsize=40)
