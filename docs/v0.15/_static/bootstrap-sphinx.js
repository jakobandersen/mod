(function($) {
	/**
	 * Patch TOC list.
	 *
	 * @param $span: Span containing nested UL"s to mutate.
	 * @param minLevel: Starting level for nested lists. (1: global, 2: local).
	 */
	var patchToc = function($ul, minLevel) {
		// $ul is our ul, but the toctree then has another ul, pull its li up
		console.assert($ul.children().length == 1, "")
		console.assert($($ul.children()[0]).prop("tagName") == "UL", "");
		$ul.find("> ul > li").each(function(index, li) {
			$li = $(li);
			$li.detach();
			$ul.append($li);
		});
		$ul.children("ul").remove();

		var $ulTop = $ul;
		var fixUL;
		fixUL = function($ul) {
			$ul.addClass("dropdown-menu");
			$ul.find("> li > a").each(function(index, a) {
				$(a).addClass("dropdown-item");
			});
			// TODO: handle sub-menues if the toc is larger
			// see e.g., https://stackoverflow.com/questions/44467377/bootstrap-4-multilevel-dropdown-inside-navigation
		};
		fixUL($ul);
	};

	$(window).on("load", function () {
		/*
		 * Scroll the window to avoid the topnav bar
		 * https://github.com/twbs/bootstrap/issues/1768
		 */
		/*
		if ($("#navbar.navbar-fixed-top").length > 0) {
			var navHeight = $("#navbar").height(),
				shiftWindow = function() { scrollBy(0, -navHeight - 10); };

			if (location.hash) {
				setTimeout(shiftWindow, 1);
			}

			window.addEventListener("hashchange", shiftWindow);
		}
		*/
	});

	$(document).ready(function () {
		// Add styling, structure to TOC"s.
		/*
		$(".dropdown-menu").each(function () {
			$(this).find("ul").each(function (index, item){
				var $item = $(item);
				$item.addClass("unstyled");
			});
		});
		*/

		// Global TOC.
		if($("ul.globaltoc li").length) {
			patchToc($("ul.globaltoc"), 1);
		} else {
			// Remove Global TOC.
			$(".globaltoc-container").remove();
		}

		// Local TOC ??
		/*
		$(".bs-sidenav ul").addClass("nav nav-list");
		$(".bs-sidenav > ul > li > a").addClass("nav-header");

		
		// back to top
		setTimeout(function () {
			var $sideBar = $(".bs-sidenav");
			var $content = $(".content");

			// Enlarge content if sidebar is larger.
			if ($sideBar.outerHeight(true) > $content.outerHeight(true)) {
				$content.css("min-height", $sideBar.outerHeight(true));
			}

			$sideBar
				// Add affix.
				.affix({
					offset: {
						top: function () {
							var offsetTop			= $sideBar.offset().top;
							var sideBarMargin	= parseInt($sideBar.css("margin-top"), 10);
							var navOuterHeight = $("#navbar").outerHeight(true);

							return (this.top = offsetTop - navOuterHeight);
						},
						bottom: function () {
							return (this.bottom = $(".footer").outerHeight(true));
						}
					}
				})
				// Trigger to reset if page content is scrolled to bottom.
				.trigger("scroll.bs.affix.data-api");
		}, 0);
		
		*/

		// Local TOC.
		$ul = $("ul.localtoc");
		patchToc($ul, 2);
		// convert second level of local toc to after a divider instead,
		// and remove subsequent levels
		console.assert($ul.children().length == 1, "");
		$ul.append('<div class="dropdown-divider"></div>');
		$ul.find("> li > ul > li").each(function(index, li) {
			$li = $(li);
			$li.find("> ul").remove();
			$li.children("a").addClass("dropdown-item");
			$li.detach();
			$ul.append($li);
		});

		// Patch all tables to remove ``docutils`` class and add Bootstrap base ``table`` class.
		$tables = $("table.docutils")
			.removeClass("docutils")
			.removeClass("align-default")
			.addClass("table")
			.addClass("table-hover");
		$tables.find("thead").addClass("thead-light");

		// Add Note, Warning styles. (BS v4 compatible).
		$(".admonition").addClass("alert alert-info")
			.filter(".warning, .caution")
				.removeClass("alert-info")
				.addClass("alert-warning").end()
			.filter(".error, .danger")
				.removeClass("alert-info")
				.addClass("alert-danger alert-error").end();

		return;

		// Inline code styles to Bootstrap style.
		$("tt.docutils.literal").not(".xref").each(function(i, e) {
			// ignore references
			if(!$(e).parent().hasClass("reference")) {
				$(e).replaceWith(function() {
					return $("<code />").html($(this).html());
				});
			}});

		// Update sourcelink to remove outerdiv (fixes appearance in navbar).
		var $srcLink = $(".nav #sourcelink");
		$srcLink.parent().html($srcLink.html());
	});
}(window.$jqTheme || window.jQuery));