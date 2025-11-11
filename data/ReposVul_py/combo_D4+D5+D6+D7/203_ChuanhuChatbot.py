# -*- coding:utf-8 -*-
import os
import logging
import sys

import gradio as gr

from modules import config
from modules.config import *
from modules.utils import *
from modules.presets import *
from modules.overwrites import *
from modules.models.models import get_model


gr.Chatbot._postprocess_chat_messages = postprocess_chat_messages
gr.Chatbot.postprocess = postprocess

with open("assets/custom.css", "r", encoding="utf-8") as f:
    z = f.read()

def W():
    return get_model(access_key = my_api_key, model_name = MODELS[DEFAULT_MODEL])[0]

with gr.Blocks(theme=small_and_beautiful_theme, css=z) as demo:
    x = gr.State(load_template(get_template_names(plain=True)[0], mode=2))
    w = gr.State("")
    assert type(my_api_key)==str
    y = gr.State(my_api_key)
    v = gr.State(W)

    u = gr.State(i18n("未命名对话历史记录"))

    with gr.Row():
        gr.HTML(CHUANHU_TITLE, elem_id="app_title")
        t = gr.Markdown(get_geoip(), elem_id="status_display")
    with gr.Row(elem_id="float_display"):
        s = gr.Markdown(value="getting user info...", elem_id="user_info")

    with gr.Row().style(equal_height=True):
        with gr.Column(scale=5):
            with gr.Row():
                r = gr.Chatbot(label="Chuanhu Chat", elem_id="chuanhu_chatbot").style(height="100%")
            with gr.Row():
                with gr.Column(min_width=225, scale=12):
                    q = gr.Textbox(
                        elem_id="user_input_tb",
                        show_label=False, placeholder=i18n("在这里输入")
                    ).style(container=False)
                with gr.Column(min_width=42, scale=1):
                    p = gr.Button(value="", variant="primary", elem_id="submit_btn")
                    o = gr.Button(value="", variant="secondary", visible=False, elem_id="cancel_btn")
            with gr.Row():
                n = gr.Button(
                    i18n("🧹 新的对话"), elem_id="empty_btn"
                )
                m = gr.Button(i18n("🔄 重新生成"))
                l = gr.Button(i18n("🗑️ 删除最旧对话"))
                k = gr.Button(i18n("🗑️ 删除最新对话"))
                with gr.Row(visible=False) as j:
                    with gr.Column(min_width=20, scale=1):
                        i = gr.Button(i18n("👍"))
                    with gr.Column(min_width=20, scale=1):
                        h = gr.Button(i18n("👎"))

        with gr.Column():
            with gr.Column(min_width=50, scale=1):
                with gr.Tab(label=i18n("模型")):
                    g = gr.Textbox(
                        show_label=True,
                        placeholder=f"Your API-key...",
                        value=hide_middle_chars(y.value),
                        type="password",
                        visible=not HIDE_MY_KEY,
                        label="API-Key",
                    )
                    if multi_api_key:
                        f = gr.Markdown(i18n("多账号模式已开启，无需输入key，可直接开始对话"), elem_id="usage_display", elem_classes="insert_block")
                    else:
                        f = gr.Markdown(i18n("**发送消息** 或 **提交key** 以显示额度"), elem_id="usage_display", elem_classes="insert_block")
                    e = gr.Dropdown(
                        label=i18n("选择模型"), choices=MODELS, multiselect=False, value=MODELS[DEFAULT_MODEL], interactive=True
                    )
                    d = gr.Dropdown(
                        label=i18n("选择LoRA模型"), choices=[], multiselect=False, interactive=True, visible=False
                    )
                    with gr.Row():
                        c = gr.Checkbox(label=i18n("单轮对话"), value=False)
                        b = gr.Checkbox(label=i18n("使用在线搜索"), value=False)
                        # render_latex_checkbox = gr.Checkbox(label=i18n("渲染LaTeX公式"), value=render_latex, interactive=True, elem_id="render_latex_checkbox")
                    a = gr.Dropdown(
                        label=i18n("选择回复语言（针对搜索&索引功能）"),
                        choices=REPLY_LANGUAGES,
                        multiselect=False,
                        value=REPLY_LANGUAGES[0],
                    )
                    z = gr.Files(label=i18n("上传"), type="file")
                    y = gr.Checkbox(label=i18n("双栏pdf"), value=advance_docs["pdf"].get("two_column", False))
                    x = gr.Button(i18n("总结"))
                    # TODO: 公式ocr
                    # formula_ocr = gr.Checkbox(label=i18n("识别公式"), value=advance_docs["pdf"].get("formula_ocr", False))

                with gr.Tab(label="Prompt"):
                    w = gr.Textbox(
                        show_label=True,
                        placeholder=i18n("在这里输入System Prompt..."),
                        label="System prompt",
                        value=INITIAL_SYSTEM_PROMPT,
                        lines=10,
                    ).style(container=False)
                    with gr.Accordion(label=i18n("加载Prompt模板"), open=True):
                        with gr.Column():
                            with gr.Row():
                                with gr.Column(scale=6):
                                    v = gr.Dropdown(
                                        label=i18n("选择Prompt模板集合文件"),
                                        choices=get_template_names(plain=True),
                                        multiselect=False,
                                        value=get_template_names(plain=True)[0],
                                    ).style(container=False)
                                with gr.Column(scale=1):
                                    u = gr.Button(i18n("🔄 刷新"))
                            with gr.Row():
                                with gr.Column():
                                    t = gr.Dropdown(
                                        label=i18n("从Prompt模板中加载"),
                                        choices=load_template(
                                            get_template_names(plain=True)[0], mode=1
                                        ),
                                        multiselect=False,
                                    ).style(container=False)

                with gr.Tab(label=i18n("保存/加载")):
                    with gr.Accordion(label=i18n("保存/加载对话历史记录"), open=True):
                        with gr.Column():
                            with gr.Row():
                                with gr.Column(scale=6):
                                    s = gr.Dropdown(
                                        label=i18n("从列表中加载对话"),
                                        choices=get_history_names(plain=True),
                                        multiselect=False
                                    )
                                with gr.Column(scale=1):
                                    r = gr.Button(i18n("🔄 刷新"))
                            with gr.Row():
                                with gr.Column(scale=6):
                                    q = gr.Textbox(
                                        show_label=True,
                                        placeholder=i18n("设置文件名: 默认为.json，可选为.md"),
                                        label=i18n("设置保存文件名"),
                                        value=i18n("对话历史记录"),
                                    ).style(container=True)
                                with gr.Column(scale=1):
                                    p = gr.Button(i18n("💾 保存对话"))
                                    o = gr.Button(i18n("📝 导出为Markdown"))
                                    gr.Markdown(i18n("默认保存于history文件夹"))
                            with gr.Row():
                                with gr.Column():
                                    n = gr.File(interactive=True)

                with gr.Tab(label=i18n("高级")):
                    gr.Markdown(i18n("# ⚠️ 务必谨慎更改 ⚠️\n\n如果无法使用请恢复默认设置"))
                    gr.HTML(APPEARANCE_SWITCHER, elem_classes="insert_block")
                    m = gr.Checkbox(
                            label=i18n("实时传输回答"), value=True, visible=ENABLE_STREAMING_OPTION
                        )
                    with gr.Accordion(i18n("参数"), open=False):
                        l = gr.Slider(
                            minimum=-0,
                            maximum=2.0,
                            value=1.0,
                            step=0.1,
                            interactive=True,
                            label="temperature",
                        )
                        k = gr.Slider(
                            minimum=-0,
                            maximum=1.0,
                            value=1.0,
                            step=0.05,
                            interactive=True,
                            label="top-p",
                        )
                        j = gr.Slider(
                            minimum=1,
                            maximum=10,
                            value=1,
                            step=1,
                            interactive=True,
                            label="n choices",
                        )
                        i = gr.Textbox(
                            show_label=True,
                            placeholder=i18n("在这里输入停止符，用英文逗号隔开..."),
                            label="stop",
                            value="",
                            lines=1,
                        )
                        h = gr.Slider(
                            minimum=1,
                            maximum=32768,
                            value=2000,
                            step=1,
                            interactive=True,
                            label="max context",
                        )
                        g = gr.Slider(
                            minimum=1,
                            maximum=32768,
                            value=1000,
                            step=1,
                            interactive=True,
                            label="max generations",
                        )
                        f = gr.Slider(
                            minimum=-2.0,
                            maximum=2.0,
                            value=0.0,
                            step=0.01,
                            interactive=True,
                            label="presence penalty",
                        )
                        e = gr.Slider(
                            minimum=-2.0,
                            maximum=2.0,
                            value=0.0,
                            step=0.01,
                            interactive=True,
                            label="frequency penalty",
                        )
                        d = gr.Textbox(
                            show_label=True,
                            placeholder=f"word:likelihood",
                            label="logit bias",
                            value="",
                            lines=1,
                        )
                        c = gr.Textbox(
                            show_label=True,
                            placeholder=i18n("用于定位滥用行为"),
                            label=i18n("用户名"),
                            value=w.value,
                            lines=1,
                        )

                    with gr.Accordion(i18n("网络设置"), open=False):
                        # 优先展示自定义的api_host
                        b = gr.Textbox(
                            show_label=True,
                            placeholder=i18n("在这里输入API-Host..."),
                            label="API-Host",
                            value=config.api_host or shared.API_HOST,
                            lines=1,
                        )
                        a = gr.Button(i18n("🔄 切换API地址"))
                        z = gr.Textbox(
                            show_label=True,
                            placeholder=i18n("在这里输入代理地址..."),
                            label=i18n("代理地址（示例：http://127.0.0.1:10809）"),
                            value="",
                            lines=2,
                        )
                        y = gr.Button(i18n("🔄 设置代理地址"))
                        x = gr.Button(i18n("🔙 恢复默认设置"))

    gr.Markdown(CHUANHU_DESCRIPTION, elem_id="description")
    gr.HTML(FOOTER.format(versions=versions_html()), elem_id="footer")

    # https://github.com/gradio-app/gradio/pull/3296
    def create_greeting(request: gr.Request):
        if hasattr(request, "username") and request.username: # is not None or is not ""
            logging.info(f"Get User Name: {request.username}")
            s, w = gr.Markdown.update(value=f"User: {request.username}"), request.username
        else:
            s, w = gr.Markdown.update(value=f"", visible=False), ""
        v = get_model(access_key = my_api_key, model_name = MODELS[DEFAULT_MODEL])[0]
        v.set_user_identifier(w)
        r = gr.Chatbot.update(label=MODELS[DEFAULT_MODEL])
        return s, w, v, toggle_like_btn_visibility(DEFAULT_MODEL), *v.auto_load(), get_history_names(False, w), r
    demo.load(create_greeting, inputs=None, outputs=[s, w, v, j, w, r, s, r], api_name="load")
    chatgpt_predict_args = dict(
        fn=predict,
        inputs=[
            v,
            w,
            r,
            m,
            b,
            z,
            a,
        ],
        outputs=[r, t],
        show_progress=True,
    )

    start_outputing_args = dict(
        fn=start_outputing,
        inputs=[],
        outputs=[p, o],
        show_progress=True,
    )

    end_outputing_args = dict(
        fn=end_outputing, inputs=[], outputs=[p, o]
    )

    reset_textbox_args = dict(
        fn=reset_textbox, inputs=[], outputs=[q]
    )

    transfer_input_args = dict(
        fn=transfer_input, inputs=[q], outputs=[w, q, p, o], show_progress=True
    )

    get_usage_args = dict(
        fn=billing_info, inputs=[v], outputs=[f], show_progress=False
    )

    load_history_from_file_args = dict(
        fn=load_chat_history,
        inputs=[v, s, w],
        outputs=[q, w, r]
    )


    # Chatbot
    o.click(interrupt, [v], [])

    q.submit(**transfer_input_args).then(**chatgpt_predict_args).then(**end_outputing_args)
    q.submit(**get_usage_args)

    p.click(**transfer_input_args).then(**chatgpt_predict_args, api_name="predict").then(**end_outputing_args)
    p.click(**get_usage_args)

    z.change(handle_file_upload, [v, z, r, a], [z, r, t])
    x.click(handle_summarize_index, [v, z, r, a], [r, t])

    n.click(
        reset,
        inputs=[v],
        outputs=[r, t],
        show_progress=True,
    )

    m.click(**start_outputing_args).then(
        retry,
        [
            v,
            r,
            m,
            b,
            z,
            a,
        ],
        [r, t],
        show_progress=True,
    ).then(**end_outputing_args)
    m.click(**get_usage_args)

    l.click(
        delete_first_conversation,
        [v],
        [t],
    )

    k.click(
        delete_last_conversation,
        [v, r],
        [r, t],
        show_progress=False
    )

    i.click(
        like,
        [v],
        [t],
        show_progress=False
    )

    h.click(
        dislike,
        [v],
        [t],
        show_progress=False
    )

    y.change(update_doc_config, [y], None)

    # LLM Models
    g.change(set_key, [v, g], [y, t], api_name="set_key").then(**get_usage_args)
    g.submit(**get_usage_args)
    c.change(set_single_turn, [v, c], None)
    e.change(get_model, [e, d, y, l, k, w, w], [v, t, r, d], show_progress=True, api_name="get_model")
    e.change(toggle_like_btn_visibility, [e], [j], show_progress=False)
    d.change(get_model, [e, d, y, l, k, w, w], [v, t, r], show_progress=True)

    # Template
    w.change(set_system_prompt, [v, w], None)
    u.click(get_template_names, None, [v])
    v.change(
        load_template,
        [v],
        [x, t],
        show_progress=True,
    )
    t.change(
        get_template_content,
        [x, t, w],
        [w],
        show_progress=True,
    )

    # S&L
    p.click(
        save_chat_history,
        [v, q, r, w],
        n,
        show_progress=True,
    )
    p.click(get_history_names, [gr.State(False), w], [s])
    o.click(
        export_markdown,
        [v, q, r, w],
        n,
        show_progress=True,
    )
    r.click(get_history_names, [gr.State(False), w], [s])
    s.change(**load_history_from_file_args)
    n.change(upload_chat_history, [v, n, w], [q, w, r])

    # Advanced
    h.change(set_token_upper_limit, [v, h], None)
    l.change(set_temperature, [v, l], None)
    k.change(set_top_p, [v, k], None)
    j.change(set_n_choices, [v, j], None)
    i.change(set_stop_sequence, [v, i], None)
    g.change(set_max_tokens, [v, g], None)
    f.change(set_presence_penalty, [v, f], None)
    e.change(set_frequency_penalty, [v, e], None)
    d.change(set_logit_bias, [v, d], None)
    c.change(set_user_identifier, [v, c], None)

    x.click(
        reset_default, [], [b, z, t], show_progress=True
    )
    a.click(
        change_api_host,
        [b],
        [t],
        show_progress=True,
    )
    y.click(
        change_proxy,
        [z],
        [t],
        show_progress=True,
    )

logging.info(
    colorama.Back.GREEN
    + "\n川虎的温馨提示：访问 http://localhost:7860 查看界面"
    + colorama.Style.RESET_ALL
)
# 默认开启本地服务器，默认可以直接从IP访问，默认不创建公开分享链接
demo.title = i18n("川虎Chat 🚀")

if __name__ == "__main__":
    reload_javascript()
    demo.queue(concurrency_count=CONCURRENT_COUNT).launch(
        blocked_paths=["config.json"],
        server_name=server_name,
        server_port=server_port,
        share=share,
        auth=auth_list if authflag else None,
        favicon_path="./assets/favicon.ico",
        inbrowser=not dockerflag, # 禁止在docker下开启inbrowser
    )
    # demo.queue(concurrency_count=CONCURRENT_COUNT).launch(server_name="0.0.0.0", server_port=7860, share=False) # 可自定义端口
    # demo.queue(concurrency_count=CONCURRENT_COUNT).launch(server_name="0.0.0.0", server_port=7860,auth=("在这里填写用户名", "在这里填写密码")) # 可设置用户名与密码
    # demo.queue(concurrency_count=CONCURRENT_COUNT).launch(auth=("在这里填写用户名", "在这里填写密码")) # 适合Nginx反向代理