"""MkDocs transforms shared by authored and generated Sail book pages."""

from __future__ import annotations

from mkdocs.config.defaults import MkDocsConfig
from mkdocs.plugins import BasePlugin
from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page

from ._eips import link_eip_references


class SailBookPlugin(BasePlugin):
    """Add canonical EIP links to prose throughout the specification book."""

    def on_page_markdown(
        self,
        markdown: str,
        /,
        *,
        page: Page,
        config: MkDocsConfig,
        files: Files,
    ) -> str:
        del page, config, files
        return link_eip_references(markdown, set(), hover=True)
