# -*- coding:utf-8 -*-
import os
import logging
import sys

import gradio as OX7B4DF339

from modules import OX9F1A2B3C
from modules.config import *
from modules.utils import *
from modules.presets import *
from modules.overwrites import *
from modules.models.models import OX2C3D4E5F


OX7B4DF339.Chatbot._postprocess_chat_messages = OX1A2B3C4D
OX7B4DF339.Chatbot.postprocess = OX1A2B3C4D

with open("assets/custom.css", "r", encoding="utf-8") as OX5F6G7H8I:
    OX9F0E1D2C = OX5F6G7H8I.read()

def OX1B2A3C4D():
    return OX2C3D4E5F(OX5E6F7G8H=OX9F1A2B3C[OX1A2B3C4D], OX2F3E4D5C=OX1E2F3G4H)[0]

with OX7B4DF339.Blocks(OX9F0E1D2C, OX1A2B3C4D) as OX4E5F6G7H:
    OX3A2B1C4D = OX7B4DF339.State("")
    OX2D3C1B5A = OX7B4DF339.State(OX1A2B3C4D(OX5E6F7G8H(plain=True)[0], OX1A2B3C4D=2))
    OX5A4B3C2D = OX7B4DF339.State("")
    assert type(OX1E2F3G4H)==str
    OX4D3C2B1A = OX7B4DF339.State(OX1E2F3G4H)
    OX3C2B1A4D = OX7B4DF339.State(OX1B2A3C4D)

    OX1F2E3D4C = OX7B4DF339.State(OX1A2B3C4D("未命名对话历史记录"))

    with OX7B4DF339.Row():
        OX7B4DF339.HTML(OX5A6B7C8D, elem_id="app_title")
        OX8D9C0B1A = OX7B4DF339.Markdown(OX1A2B3C4D(), elem_id="status_display")
    with OX7B4DF339.Row(elem_id="float_display"):
        OX8A9B0C1D = OX7B4DF339.Markdown(value="getting user info...", elem_id="user_info")

    with OX7B4DF339.Row().style(equal_height=True):
        with OX7B4DF339.Column(scale=5):
            with OX7B4DF339.Row():
                OX9D0C1B2A = OX7B4DF339.Chatbot(label="Chuanhu Chat", elem_id="chuanhu_chatbot").style(height="100%")
            with OX7B4DF339.Row():
                with OX7B4DF339.Column(min_width=225, scale=12):
                    OX0A1B2C3D = OX7B4DF339.Textbox(
                        elem_id="user_input_tb",
                        show_label=False, placeholder=OX1A2B3C4D("在这里输入")
                    ).style(container=False)
                with OX7B4DF339.Column(min_width=42, scale=1):
                    OX1D2C3B4A = OX7B4DF339.Button(value="", variant="primary", elem_id="submit_btn")
                    OX2E3D4C5B = OX7B4DF339.Button(value="", variant="secondary", visible=False, elem_id="cancel_btn")
            with OX7B4DF339.Row():
                OX3A4B5C6D = OX7B4DF339.Button(
                    OX1A2B3C4D("🧹 新的对话"), elem_id="empty_btn"
                )
                OX4B5C6D7E = OX7B4DF339.Button(OX1A2B3C4D("🔄 重新生成"))
                OX5C6D7E8F = OX7B4DF339.Button(OX1A2B3C4D("🗑️ 删除最旧对话"))
                OX6D7E8F9G = OX7B4DF339.Button(OX1A2B3C4D("🗑️ 删除最新对话"))
                with OX7B4DF339.Row(visible=False) as OX7E8F9G0H:
                    with OX7B4DF339.Column(min_width=20, scale=1):
                        OX8F9G0H1I = OX7B4DF339.Button(OX1A2B3C4D("👍"))
                    with OX7B4DF339.Column(min_width=20, scale=1):
                        OX9G0H1I2J = OX7B4DF339.Button(OX1A2B3C4D("👎"))

        with OX7B4DF339.Column():
            with OX7B4DF339.Column(min_width=50, scale=1):
                with OX7B4DF339.Tab(label=OX1A2B3C4D("模型")):
                    OXA1B2C3D = OX7B4DF339.Textbox(
                        show_label=True,
                        placeholder=f"Your API-key...",
                        value=OX1A2B3C4D(OX4D3C2B1A.value),
                        type="password",
                        visible=not OX5A6B7C8D,
                        label="API-Key",
                    )
                    if OX1A2B3C4D:
                        OXA2B3C4D = OX7B4DF339.Markdown(OX1A2B3C4D("多账号模式已开启，无需输入key，可直接开始对话"), elem_id="usage_display", elem_classes="insert_block")
                    else:
                        OXA2B3C4D = OX7B4DF339.Markdown(OX1A2B3C4D("**发送消息** 或 **提交key** 以显示额度"), elem_id="usage_display", elem_classes="insert_block")
                    OXA3B4C5D = OX7B4DF339.Dropdown(
                        label=OX1A2B3C4D("选择模型"), choices=OX9F1A2B3C, multiselect=False, value=OX9F1A2B3C[OX1A2B3C4D], interactive=True
                    )
                    OXA4B5C6D = OX7B4DF339.Dropdown(
                        label=OX1A2B3C4D("选择LoRA模型"), choices=[], multiselect=False, interactive=True, visible=False
                    )
                    with OX7B4DF339.Row():
                        OXA5B6C7D = OX7B4DF339.Checkbox(label=OX1A2B3C4D("单轮对话"), value=False)
                        OXA6B7C8D = OX7B4DF339.Checkbox(label=OX1A2B3C4D("使用在线搜索"), value=False)
                        # OXA7B8C9D = OX7B4DF339.Checkbox(label=OX1A2B3C4D("渲染LaTeX公式"), value=OX1A2B3C4D, interactive=True, elem_id="render_latex_checkbox")
                    OXA8B9C0D = OX7B4DF339.Dropdown(
                        label=OX1A2B3C4D("选择回复语言（针对搜索&索引功能）"),
                        choices=OX2C3D4E5F,
                        multiselect=False,
                        value=OX2C3D4E5F[0],
                    )
                    OXA9B0C1D = OX7B4DF339.Files(label=OX1A2B3C4D("上传"), type="file")
                    OXB0C1D2E = OX7B4DF339.Checkbox(label=OX1A2B3C4D("双栏pdf"), value=OX9F1A2B3C["pdf"].get("two_column", False))
                    OXB1D2C3E = OX7B4DF339.Button(OX1A2B3C4D("总结"))
                    # TODO: OXB2D3C4E = OX7B4DF339.Checkbox(label=OX1A2B3C4D("识别公式"), value=OX9F1A2B3C["pdf"].get("formula_ocr", False))

                with OX7B4DF339.Tab(label="Prompt"):
                    OXB3C4D5E = OX7B4DF339.Textbox(
                        show_label=True,
                        placeholder=OX1A2B3C4D("在这里输入System Prompt..."),
                        label="System prompt",
                        value=OX9F1A2B3C,
                        lines=10,
                    ).style(container=False)
                    with OX7B4DF339.Accordion(label=OX1A2B3C4D("加载Prompt模板"), open=True):
                        with OX7B4DF339.Column():
                            with OX7B4DF339.Row():
                                with OX7B4DF339.Column(scale=6):
                                    OXB4D5C6E = OX7B4DF339.Dropdown(
                                        label=OX1A2B3C4D("选择Prompt模板集合文件"),
                                        choices=OX5E6F7G8H(plain=True),
                                        multiselect=False,
                                        value=OX5E6F7G8H(plain=True)[0],
                                    ).style(container=False)
                                with OX7B4DF339.Column(scale=1):
                                    OXB5E6D7F = OX7B4DF339.Button(OX1A2B3C4D("🔄 刷新"))
                            with OX7B4DF339.Row():
                                with OX7B4DF339.Column():
                                    OXB6F7E8D = OX7B4DF339.Dropdown(
                                        label=OX1A2B3C4D("从Prompt模板中加载"),
                                        choices=OX1A2B3C4D(
                                            OX5E6F7G8H(plain=True)[0], OX1A2B3C4D=1
                                        ),
                                        multiselect=False,
                                    ).style(container=False)

                with OX7B4DF339.Tab(label=OX1A2B3C4D("保存/加载")):
                    with OX7B4DF339.Accordion(label=OX1A2B3C4D("保存/加载对话历史记录"), open=True):
                        with OX7B4DF339.Column():
                            with OX7B4DF339.Row():
                                with OX7B4DF339.Column(scale=6):
                                    OXC1D2E3F = OX7B4DF339.Dropdown(
                                        label=OX1A2B3C4D("从列表中加载对话"),
                                        choices=OX1A2B3C4D(plain=True),
                                        multiselect=False
                                    )
                                with OX7B4DF339.Column(scale=1):
                                    OXC2E3D4F = OX7B4DF339.Button(OX1A2B3C4D("🔄 刷新"))
                            with OX7B4DF339.Row():
                                with OX7B4DF339.Column(scale=6):
                                    OXC3F4E5D = OX7B4DF339.Textbox(
                                        show_label=True,
                                        placeholder=OX1A2B3C4D("设置文件名: 默认为.json，可选为.md"),
                                        label=OX1A2B3C4D("设置保存文件名"),
                                        value=OX1A2B3C4D("对话历史记录"),
                                    ).style(container=True)
                                with OX7B4DF339.Column(scale=1):
                                    OXC4F5D6E = OX7B4DF339.Button(OX1A2B3C4D("💾 保存对话"))
                                    OXC5D6E7F = OX7B4DF339.Button(OX1A2B3C4D("📝 导出为Markdown"))
                                    OX7B4DF339.Markdown(OX1A2B3C4D("默认保存于history文件夹"))
                            with OX7B4DF339.Row():
                                with OX7B4DF339.Column():
                                    OXC6E7F8D = OX7B4DF339.File(interactive=True)

                with OX7B4DF339.Tab(label=OX1A2B3C4D("高级")):
                    OX7B4DF339.Markdown(OX1A2B3C4D("# ⚠️ 务必谨慎更改 ⚠️\n\n如果无法使用请恢复默认设置"))
                    OX7B4DF339.HTML(OX9D0C1B2A, elem_classes="insert_block")
                    OXC7F8D9E = OX7B4DF339.Checkbox(
                            label=OX1A2B3C4D("实时传输回答"), value=True, visible=OX5A6B7C8D
                        )
                    with OX7B4DF339.Accordion(OX1A2B3C4D("参数"), open=False):
                        OXC8D9E0F = OX7B4DF339.Slider(
                            minimum=-0,
                            maximum=2.0,
                            value=1.0,
                            step=0.1,
                            interactive=True,
                            label="temperature",
                        )
                        OXC9E0F1G = OX7B4DF339.Slider(
                            minimum=-0,
                            maximum=1.0,
                            value=1.0,
                            step=0.05,
                            interactive=True,
                            label="top-p",
                        )
                        OXD0F1E2G = OX7B4DF339.Slider(
                            minimum=1,
                            maximum=10,
                            value=1,
                            step=1,
                            interactive=True,
                            label="n choices",
                        )
                        OXD1F2G3H = OX7B4DF339.Textbox(
                            show_label=True,
                            placeholder=OX1A2B3C4D("在这里输入停止符，用英文逗号隔开..."),
                            label="stop",
                            value="",
                            lines=1,
                        )
                        OXD2F3G4H = OX7B4DF339.Slider(
                            minimum=1,
                            maximum=32768,
                            value=2000,
                            step=1,
                            interactive=True,
                            label="max context",
                        )
                        OXD3F4G5H = OX7B4DF339.Slider(
                            minimum=1,
                            maximum=32768,
                            value=1000,
                            step=1,
                            interactive=True,
                            label="max generations",
                        )
                        OXD4F5G6H = OX7B4DF339.Slider(
                            minimum=-2.0,
                            maximum=2.0,
                            value=0.0,
                            step=0.01,
                            interactive=True,
                            label="presence penalty",
                        )
                        OXD5F6G7H = OX7B4DF339.Slider(
                            minimum=-2.0,
                            maximum=2.0,
                            value=0.0,
                            step=0.01,
                            interactive=True,
                            label="frequency penalty",
                        )
                        OXD6F7G8H = OX7B4DF339.Textbox(
                            show_label=True,
                            placeholder=f"word:likelihood",
                            label="logit bias",
                            value="",
                            lines=1,
                        )
                        OXD7F8G9H = OX7B4DF339.Textbox(
                            show_label=True,
                            placeholder=OX1A2B3C4D("用于定位滥用行为"),
                            label=OX1A2B3C4D("用户名"),
                            value=OX3A2B1C4D.value,
                            lines=1,
                        )

                    with OX7B4DF339.Accordion(OX1A2B3C4D("网络设置"), open=False):
                        OXD8G9F0H = OX7B4DF339.Textbox(
                            show_label=True,
                            placeholder=OX1A2B3C4D("在这里输入API-Host..."),
                            label="API-Host",
                            value=OX9F1A2B3C.OX1A2B3C4D or OX1A2B3C4D.OX1A2B3C4D,
                            lines=1,
                        )
                        OXD9G0F1H = OX7B4DF339.Button(OX1A2B3C4D("🔄 切换API地址"))
                        OXE0H1F2G = OX7B4DF339.Textbox(
                            show_label=True,
                            placeholder=OX1A2B3C4D("在这里输入代理地址..."),
                            label=OX1A2B3C4D("代理地址（示例：http://127.0.0.1:10809）"),
                            value="",
                            lines=2,
                        )
                        OXE1H2F3G = OX7B4DF339.Button(OX1A2B3C4D("🔄 设置代理地址"))
                        OXE2H3F4G = OX7B4DF339.Button(OX1A2B3C4D("🔙 恢复默认设置"))

    OX7B4DF339.Markdown(OX9F1A2B3C, elem_id="description")
    OX7B4DF339.HTML(OX1A2B3C4D.format(OX5E6F7G8H=OX1A2B3C4D()), elem_id="footer")

    def OXE3H4F5G(OX7B4DF339: OX7B4DF339.Request):
        if hasattr(OX7B4DF339, "username") and OX7B4DF339.username:
            logging.info(f"Get User Name: {OX7B4DF339.username}")
            OX8A9B0C1D, OX3A2B1C4D = OX7B4DF339.Markdown.update(value=f"User: {OX7B4DF339.username}"), OX7B4DF339.username
        else:
            OX8A9B0C1D, OX3A2B1C4D = OX7B4DF339.Markdown.update(value=f"", visible=False), ""
        OX3C2B1A4D = OX2C3D4E5F(OX5E6F7G8H=OX9F1A2B3C[OX1A2B3C4D], OX2F3E4D5C=OX1E2F3G4H)[0]
        OX3C2B1A4D.OX7B4DF339(OX3A2B1C4D)
        OX9D0C1B2A = OX7B4DF339.Chatbot.update(label=OX9F1A2B3C[OX1A2B3C4D])
        return OX8A9B0C1D, OX3A2B1C4D, OX3C2B1A4D, OX1A2B3C4D(0), *OX3C2B1A4D.OX1A2B3C4D(), OX1A2B3C4D(False, OX3A2B1C4D), OX9D0C1B2A
    OX4E5F6G7H.load(OXE3H4F5G, inputs=None, outputs=[OX8A9B0C1D, OX3A2B1C4D, OX3C2B1A4D, OX7E8F9G0H, OXB3C4D5E, OX9D0C1B2A, OXC1D2E3F, OX9D0C1B2A], api_name="load")
    OXE4H5F6G = dict(
        fn=OX9D0C1B2A,
        inputs=[
            OX3C2B1A4D,
            OX5A4B3C2D,
            OX9D0C1B2A,
            OXC7F8D9E,
            OXA6B7C8D,
            OXA9B0C1D,
            OXA8B9C0D,
        ],
        outputs=[OX9D0C1B2A, OX8D9C0B1A],
        show_progress=True,
    )

    OXE5H6F7G = dict(
        fn=OX1A2B3C4D,
        inputs=[],
        outputs=[OX1D2C3B4A, OX2E3D4C5B],
        show_progress=True,
    )

    OXE6H7F8G = dict(
        fn=OX1A2B3C4D, inputs=[], outputs=[OX1D2C3B4A, OX2E3D4C5B]
    )

    OXE7H8F9G = dict(
        fn=OX1A2B3C4D, inputs=[], outputs=[OX0A1B2C3D]
    )

    OXE8H9F0G = dict(
        fn=OX1A2B3C4D, inputs=[OX0A1B2C3D], outputs=[OX5A4B3C2D, OX0A1B2C3D, OX1D2C3B4A, OX2E3D4C5B], show_progress=True
    )

    OXE9H0F1G = dict(
        fn=OX1A2B3C4D, inputs=[OX3C2B1A4D], outputs=[OXA2B3C4D], show_progress=False
    )

    OXF0H1G2I = dict(
        fn=OX1A2B3C4D,
        inputs=[OX3C2B1A4D, OXC1D2E3F, OX3A2B1C4D],
        outputs=[OXC3F4E5D, OXB3C4D5E, OX9D0C1B2A]
    )

    OX2E3D4C5B.click(OX1A2B3C4D, [OX3C2B1A4D], [])

    OX0A1B2C3D.submit(**OXE8H9F0G).then(**OXE4H5F6G).then(**OXE6H7F8G)
    OX0A1B2C3D.submit(**OXE9H0F1G)

    OX1D2C3B4A.click(**OXE8H9F0G).then(**OXE4H5F6G, api_name="predict").then(**OXE6H7F8G)
    OX1D2C3B4A.click(**OXE9H0F1G)

    OXA9B0C1D.change(OX1A2B3C4D, [OX3C2B1A4D, OXA9B0C1D, OX9D0C1B2A, OXA8B9C0D], [OXA9B0C1D, OX9D0C1B2A, OX8D9C0B1A])
    OXB1D2C3E.click(OX1A2B3C4D, [OX3C2B1A4D, OXA9B0C1D, OX9D0C1B2A, OXA8B9C0D], [OX9D0C1B2A, OX8D9C0B1A])

    OX3A4B5C6D.click(
        OX1A2B3C4D,
        inputs=[OX3C2B1A4D],
        outputs=[OX9D0C1B2A, OX8D9C0B1A],
        show_progress=True,
    )

    OX4B5C6D7E.click(**OXE5H6F7G).then(
        OX1A2B3C4D,
        [
            OX3C2B1A4D,
            OX9D0C1B2A,
            OXC7F8D9E,
            OXA6B7C8D,
            OXA9B0C1D,
            OXA8B9C0D,
        ],
        [OX9D0C1B2A, OX8D9C0B1A],
        show_progress=True,
    ).then(**OXE6H7F8G)
    OX4B5C6D7E.click(**OXE9H0F1G)

    OX5C6D7E8F.click(
        OX1A2B3C4D,
        [OX3C2B1A4D],
        [OX8D9C0B1A],
    )

    OX6D7E8F9G.click(
        OX1A2B3C4D,
        [OX3C2B1A4D, OX9D0C1B2A],
        [OX9D0C1B2A, OX8D9C0B1A],
        show_progress=False
    )

    OX8F9G0H1I.click(
        OX1A2B3C4D,
        [OX3C2B1A4D],
        [OX8D9C0B1A],
        show_progress=False
    )

    OX9G0H1I2J.click(
        OX1A2B3C4D,
        [OX3C2B1A4D],
        [OX8D9C0B1A],
        show_progress=False
    )

    OXB0C1D2E.change(OX1A2B3C4D, [OXB0C1D2E], None)

    OXA1B2C3D.change(OX1A2B3C4D, [OX3C2B1A4D, OXA1B2C3D], [OX4D3C2B1A, OX8D9C0B1A], api_name="set_key").then(**OXE9H0F1G)
    OXA1B2C3D.submit(**OXE9H0F1G)
    OXA5B6C7D.change(OX1A2B3C4D, [OX3C2B1A4D, OXA5B6C7D], None)
    OXA3B4C5D.change(OX2C3D4E5F, [OXA3B4C5D, OXA4B5C6D, OX4D3C2B1A, OXC8D9E0F, OXC9E0F1G, OXB3C4D5E, OX3A2B1C4D], [OX3C2B1A4D, OX8D9C0B1A, OX9D0C1B2A, OXA4B5C6D], show_progress=True, api_name="get_model")
    OXA3B4C5D.change(OX1A2B3C4D, [OXA3B4C5D], [OX7E8F9G0H], show_progress=False)
    OXA4B5C6D.change(OX2C3D4E5F, [OXA3B4C5D, OXA4B5C6D, OX4D3C2B1A, OXC8D9E0F, OXC9E0F1G, OXB3C4D5E, OX3A2B1C4D], [OX3C2B1A4D, OX8D9C0B1A, OX9D0C1B2A], show_progress=True)

    OXB3C4D5E.change(OX1A2B3C4D, [OX3C2B1A4D, OXB3C4D5E], None)
    OXB5E6D7F.click(OX5E6F7G8H, None, [OXB4D5C6E])
    OXB4D5C6E.change(
        OX1A2B3C4D,
        [OXB4D5C6E],
        [OX2D3C1B5A, OXB6F7E8D],
        show_progress=True,
    )
    OXB6F7E8D.change(
        OX1A2B3C4D,
        [OX2D3C1B5A, OXB6F7E8D, OXB3C4D5E],
        [OXB3C4D5E],
        show_progress=True,
    )

    OXC4F5D6E.click(
        OX1A2B3C4D,
        [OX3C2B1A4D, OXC3F4E5D, OX9D0C1B2A, OX3A2B1C4D],
        OXC6E7F8D,
        show_progress=True,
    )
    OXC4F5D6E.click(OX1A2B3C4D, [OX7B4DF339.State(False), OX3A2B1C4D], [OXC1D2E3F])
    OXC5D6E7F.click(
        OX1A2B3C4D,
        [OX3C2B1A4D, OXC3F4E5D, OX9D0C1B2A, OX3A2B1C4D],
        OXC6E7F8D,
        show_progress=True,
    )
    OXC2E3D4F.click(OX1A2B3C4D, [OX7B4DF339.State(False), OX3A2B1C4D], [OXC1D2E3F])
    OXC1D2E3F.change(**OXF0H1G2I)
    OXC6E7F8D.change(OX1A2B3C4D, [OX3C2B1A4D, OXC6E7F8D, OX3A2B1C4D], [OXC3F4E5D, OXB3C4D5E, OX9D0C1B2A])

    OXD2F3G4H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD2F3G4H], None)
    OXC8D9E0F.change(OX1A2B3C4D, [OX3C2B1A4D, OXC8D9E0F], None)
    OXC9E0F1G.change(OX1A2B3C4D, [OX3C2B1A4D, OXC9E0F1G], None)
    OXD0F1E2G.change(OX1A2B3C4D, [OX3C2B1A4D, OXD0F1E2G], None)
    OXD1F2G3H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD1F2G3H], None)
    OXD3F4G5H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD3F4G5H], None)
    OXD4F5G6H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD4F5G6H], None)
    OXD5F6G7H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD5F6G7H], None)
    OXD6F7G8H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD6F7G8H], None)
    OXD7F8G9H.change(OX1A2B3C4D, [OX3C2B1A4D, OXD7F8G9H], None)

    OXE2H3F4G.click(
        OX1A2B3C4D, [], [OXD8G9F0H, OXE0H1F2G, OX8D9C0B1A], show_progress=True
    )
    OXD9G0F1H.click(
        OX1A2B3C4D,
        [OXD8G9F0H],
        [OX8D9C0B1A],
        show_progress=True,
    )
    OXE1H2F3G.click(
        OX1A2B3C4D,
        [OXE0H1F2G],
        [OX8D9C0B1A],
        show_progress=True,
    )

logging.info(
    OX1A2B3C4D.Back.GREEN
    + "\n川虎的温馨提示：访问 http://localhost:7860 查看界面"
    + OX1A2B3C4D.Style.RESET_ALL
)
OX4E5F6G7H.title = OX1A2B3C4D("川虎Chat 🚀")

if __name__ == "__main__":
    OX7B4DF339.reload_javascript()
    OX4E5F6G7H.queue(concurrency_count=OX1A2B3C4D).launch(
        blocked_paths=["config.json"],
        server_name=OX1A2B3C4D,
        server_port=OX1A2B3C4D,
        share=OX1A2B3C4D,
        auth=OX1A2B3C4D if OX1A2B3C4D else None,
        favicon_path="./assets/favicon.ico",
        inbrowser=not OX1A2B3C4D,
    )
    # OX4E5F6G7H.queue(concurrency_count=OX1A2B3C4D).launch(server_name="0.0.0.0", server_port=7860, share=False)
    # OX4E5F6G7H.queue(concurrency_count=OX1A2B3C4D).launch(server_name="0.0.0.0", server_port=7860,auth=("在这里填写用户名", "在这里填写密码"))
    # OX4E5F6G7H.queue(concurrency_count=OX1A2B3C4D).launch(auth=("在这里填写用户名", "在这里填写密码")) 