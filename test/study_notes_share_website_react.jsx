export default function StudyNotesWebsite() {
  const notes = [
    {
      title: "Linux 进程间通信",
      tag: "Linux",
      date: "2026-05-08",
      content:
        "总结了管道、共享内存、消息队列、信号量、socket 等 IPC 机制，以及它们的适用场景和底层原理。",
    },
    {
      title: "ROS 控制四足机器人",
      tag: "ROS",
      date: "2026-05-06",
      content:
        "记录了 legged_control 的启动流程，以及 gait 在控制器中的生效过程。",
    },
    {
      title: "LeetCode 二分法模板",
      tag: "算法",
      date: "2026-05-01",
      content:
        "整理了经典二分查找模板，包括左边界、右边界以及 lower_bound 的实现。",
    },
  ];

  return (
    <div className="min-h-screen bg-gray-100 text-gray-900">
      {/* Header */}
      <header className="bg-black text-white py-6 shadow-lg">
        <div className="max-w-6xl mx-auto px-6 flex flex-col md:flex-row md:items-center md:justify-between gap-4">
          <div>
            <h1 className="text-4xl font-bold">LIC Study Notes</h1>
            <p className="text-gray-300 mt-2">
              分享 Linux、ROS、算法、机器人与 SLAM 学习笔记
            </p>
          </div>

          <div className="flex gap-3">
            <button className="bg-white text-black px-5 py-2 rounded-2xl font-semibold hover:scale-105 transition">
              写笔记
            </button>
            <button className="border border-white px-5 py-2 rounded-2xl hover:bg-white hover:text-black transition">
              关于我
            </button>
          </div>
        </div>
      </header>

      {/* Hero */}
      <section className="max-w-6xl mx-auto px-6 py-12 grid md:grid-cols-2 gap-10 items-center">
        <div>
          <h2 className="text-5xl font-extrabold leading-tight">
            将你的学习过程
            <span className="block text-blue-600">公开成知识库</span>
          </h2>

          <p className="mt-6 text-lg text-gray-600 leading-8">
            这个网页适合用于分享 Linux、算法、机器人、SLAM、ROS、强化学习等技术学习笔记。
            可以作为个人博客、学习记录站、面试知识整理网站使用。
          </p>

          <div className="mt-8 flex gap-4">
            <button className="bg-blue-600 text-white px-6 py-3 rounded-2xl text-lg hover:scale-105 transition shadow-lg">
              开始写作
            </button>

            <button className="bg-white border px-6 py-3 rounded-2xl text-lg hover:bg-gray-50 transition">
              查看笔记
            </button>
          </div>
        </div>

        <div className="bg-white rounded-3xl p-8 shadow-2xl border">
          <div className="flex items-center justify-between mb-6">
            <h3 className="text-2xl font-bold">学习统计</h3>
            <span className="bg-green-100 text-green-700 px-3 py-1 rounded-full text-sm font-semibold">
              持续更新
            </span>
          </div>

          <div className="grid grid-cols-2 gap-5">
            <div className="bg-gray-100 rounded-2xl p-6">
              <div className="text-4xl font-bold">128</div>
              <div className="text-gray-500 mt-2">学习笔记</div>
            </div>

            <div className="bg-gray-100 rounded-2xl p-6">
              <div className="text-4xl font-bold">26</div>
              <div className="text-gray-500 mt-2">技术专题</div>
            </div>

            <div className="bg-gray-100 rounded-2xl p-6">
              <div className="text-4xl font-bold">742</div>
              <div className="text-gray-500 mt-2">代码片段</div>
            </div>

            <div className="bg-gray-100 rounded-2xl p-6">
              <div className="text-4xl font-bold">365</div>
              <div className="text-gray-500 mt-2">学习天数</div>
            </div>
          </div>
        </div>
      </section>

      {/* Search */}
      <section className="max-w-6xl mx-auto px-6">
        <div className="bg-white rounded-3xl p-6 shadow-lg border flex flex-col md:flex-row gap-4 items-center justify-between">
          <input
            type="text"
            placeholder="搜索你的学习笔记..."
            className="w-full border rounded-2xl px-5 py-3 outline-none focus:ring-2 focus:ring-blue-500"
          />

          <select className="border rounded-2xl px-5 py-3">
            <option>全部分类</option>
            <option>Linux</option>
            <option>ROS</option>
            <option>算法</option>
            <option>SLAM</option>
          </select>
        </div>
      </section>

      {/* Notes */}
      <section className="max-w-6xl mx-auto px-6 py-12">
        <div className="flex items-center justify-between mb-8">
          <h2 className="text-3xl font-bold">最新学习笔记</h2>
          <button className="text-blue-600 font-semibold hover:underline">
            查看全部
          </button>
        </div>

        <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-8">
          {notes.map((note, index) => (
            <div
              key={index}
              className="bg-white rounded-3xl shadow-lg border p-6 hover:-translate-y-2 transition duration-300"
            >
              <div className="flex items-center justify-between mb-4">
                <span className="bg-blue-100 text-blue-700 px-3 py-1 rounded-full text-sm font-semibold">
                  {note.tag}
                </span>
                <span className="text-gray-400 text-sm">{note.date}</span>
              </div>

              <h3 className="text-2xl font-bold mb-4 leading-snug">
                {note.title}
              </h3>

              <p className="text-gray-600 leading-7">{note.content}</p>

              <button className="mt-6 w-full bg-black text-white py-3 rounded-2xl hover:bg-gray-800 transition">
                阅读全文
              </button>
            </div>
          ))}
        </div>
      </section>

      {/* Footer */}
      <footer className="bg-black text-white mt-16">
        <div className="max-w-6xl mx-auto px-6 py-10 grid md:grid-cols-3 gap-10">
          <div>
            <h3 className="text-2xl font-bold">LIC Notes</h3>
            <p className="text-gray-400 mt-4 leading-7">
              一个用于记录技术成长与学习过程的个人知识网站。
            </p>
          </div>

          <div>
            <h4 className="font-bold text-lg mb-4">技术方向</h4>
            <ul className="space-y-3 text-gray-400">
              <li>Linux</li>
              <li>ROS</li>
              <li>SLAM</li>
              <li>四足机器人</li>
            </ul>
          </div>

          <div>
            <h4 className="font-bold text-lg mb-4">联系方式</h4>
            <ul className="space-y-3 text-gray-400">
              <li>GitHub</li>
              <li>Email</li>
              <li>Bilibili</li>
            </ul>
          </div>
        </div>
      </footer>
    </div>
  );
}
