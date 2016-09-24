# Labrador Jekyll Theme v1.0

This theme was built to present a little bit about of some professional profile.

## Demo
- [https://donini.github.io/labrador-jekyll-theme/](https://donini.github.io/labrador-jekyll-theme/)

## Contents

- [Pre-requisites](#pre-requisites)
- [Directory structure](#directory-structure)
    - [_config.yml](#settings-on-configyml)
    - [.htaccess](#warning)
    - [Translations](#translations)
        - [_data](#_data-folder)
        - [Social links](#social-links)
    - [Components](#components)
    - [Blog posts](#blog-posts)
    - [Media files for posts](#media-files-for-posts)
- [Author](#author)
- [License](#license)


## Pre-requisites

This theme has created with:
- [Jekyll](https://jekyllrb.com/)
- [SASS](http://sass-lang.com)
- [pygments](https://jekyllrb.com/docs/templates/)
- [syntax-highlighting](http://jekyll-windows.juthilo.com/3-syntax-highlighting/)

## Directory structure

``` ruby
labrador/
+-- .gitignore
+-- .htaccess
+-- _config.yml
+-- blog.md
+-- en.md
+-- pt.md
+-- _data
    +-- lang
    ¦   +-- [language_code].json 
    +-- social.json
+-- _includes
    +-- general
    ¦   +-- blog
    +-- sections
+-- _layouts
+-- _posts
+-- assets
    +-- css
    |   +-- blog
    |   +-- sass
    +-- fonts
    +-- img
    +-- js
+-- media
```


### Settings on _config.yml_

**title**: # the title for your site
**email**: # your e-mail address
**description**: # this means to ignore newlines until "baseurl:"
**baseurl**: # the subpath of your site, e.g. /blog
**url**:  # the base hostname & protocol for your site
**logo**: # Your logo uri

### Translations

To use multiples languages you just need create file for yout desired language with .md or .markdown extension on project root. This files need this header:
```
---
layout: default
permalink: /[CODE_OF_YOUR_LANGUAGE]/
lang: [CODE_OF_YOUR_LANGUAGE]
---
```

For default this theme has [en] for english and [pt] for portuguese.

#### _data folder
On _data/lang/* is located the languages data files, with json synthax.

Each json data group indicate in each file constains the relevant data to section information. See [sections](#sections).

To create your own translation, just duplicate a json file, rename it and translate the terms.

#### Social links

You can link your social profiles on yout site, to do this, just open _data/social.json file, and add or remove your data for your social networks.

### Components

On _includes folder are located the componentes used on this theme.

There is two folders:
* _general_ where are located the general componentes, like header, footer, menu, language change, social links and blog elements.
* _sections_ where are located the blocks of each section of front page (single-paged).

### Blog posts

The blog posts are located in _posts folder, each file, need this header:
```
---
layout: post
title:  "[THE_TITLE_OF_YOUR_POST]"
subtitle: "[THE_SUBTITLE_OF_YOUR_POST]"
date:   [POST_DATE]
tags: [TAG1],[TAG2]
image: /media/[IMAGE_THAT_REPRESENT_YOUR_POST]
lang: [CODE_OF_LANGUAGE]
permalink: /blog/:year/:month/:day/:title/ --> [THIS_PERMALINK_PATTERN_CAN_BE_CHANGE_AS_YOU_DESIRE]
---
```

The _image:_ variable is used with a placeholder in the middle of content as you desire.

### Media files for posts

This folder contains the resources for posts, like images, documents, etc.

#### Warning

> Be careful, in the theme root is located .htaccess file, this file contains a code that will either redirect access to 'pt' directory. In my case this is the address of the home page.

### License

Open sourced under the [MIT license](https://github.com/donini/labrador-jekyll-theme/blob/master/LICENSE.md).CENSE.md).